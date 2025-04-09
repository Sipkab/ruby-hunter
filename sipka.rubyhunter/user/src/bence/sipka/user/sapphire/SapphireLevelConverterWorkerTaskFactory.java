package bence.sipka.user.sapphire;

import java.io.BufferedReader;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayInputStream;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;
import saker.build.trace.BuildTrace;

public class SapphireLevelConverterWorkerTaskFactory implements TaskFactory<SapphireLevelConverterTaskFactory.Output>,
		Task<SapphireLevelConverterTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private static final byte SAPPHIRE_CMD_DEMOCOUNT = (byte) 128;
	private static final byte SAPPHIRE_CMD_PLAYERCOUNT = (byte) 129;
	private static final byte SAPPHIRE_CMD_UUID = (byte) 130;
	private static final byte SAPPHIRE_CMD_LEADERBOARDS = (byte) 131;
	private static final byte SAPPHIRE_CMD_NON_MODIFYABLE_FLAG = 'X';
	private static final byte SAPPHIRE_CMD_END_OF_FILE = (byte) 0;

	private static final int LEADERBOARD_GEMS = 0x01;
	private static final int LEADERBOARD_TIME = 0x02;
	private static final int LEADERBOARD_STEPS = 0x04;

	private static final String[] CATEGORY_MAP = { null, "Fun", "Discovery", "Action", "Battle", "Puzzle", "Science",
			"Work", "None" };
	private static final String[] DIFFICULTY_MAP = { "Tutorial", "Simple", "Easy", "Moderate", "Normal", "Tricky",
			"Tough", "Difficult", "Hard", "M.A.D.", "Unrated" };
	private static final int CATEGORY_NONE = 7;
	public static final int SAPPHIRE_RELEASE_VERSION = 3;

	private int maxDifficulty = 10;
	private Set<FileCollectionStrategy> levelcollectionstrategies;
	private Set<FileCollectionStrategy> musiccollectionstrategies;

	/**
	 * For {@link Externalizable}.
	 */
	public SapphireLevelConverterWorkerTaskFactory() {
	}

	public SapphireLevelConverterWorkerTaskFactory(int maxDifficulty,
			Set<FileCollectionStrategy> levelcollectionstrategies,
			Set<FileCollectionStrategy> musiccollectionstrategies) {
		this.maxDifficulty = maxDifficulty;
		this.levelcollectionstrategies = levelcollectionstrategies;
		this.musiccollectionstrategies = musiccollectionstrategies;
	}

	@Override
	public Task<? extends SapphireLevelConverterTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public SapphireLevelConverterTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(SapphireLevelConverterTaskFactory.TASK_NAME);

		ConverterContext cc = new ConverterContext();

		SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(SapphireLevelConverterTaskFactory.TASK_NAME);
		genDirectory.clear();

		NavigableMap<SakerPath, SakerFile> levelinputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, levelcollectionstrategies);

		NavigableMap<SakerPath, SakerFile> musicinputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, musiccollectionstrategies);

		for (Entry<SakerPath, SakerFile> entry : musicinputfiles.entrySet()) {
			this.parseMusicFile(entry.getValue(), cc);
		}

		NavigableMap<String, SakerPath> assets = new TreeMap<>();

		SakerPath workingdirpath = taskcontext.getTaskWorkingDirectoryPath();

		for (Entry<SakerPath, SakerFile> entry : levelinputfiles.entrySet()) {
			SakerFile convertresult = convertFile(taskcontext, entry.getKey(), entry.getValue(), genDirectory,
					workingdirpath, cc);
			if (convertresult != null) {
				assets.put(workingdirpath.relativize(entry.getKey()).toString(), convertresult.getSakerPath());
			}
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(genDirectory
				.getFilesRecursiveByPath(genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		genDirectory.synchronize();

		SapphireLevelConverterTaskFactory.Output result = new SapphireLevelConverterTaskFactory.Output(assets);
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	private SakerFile convertFile(TaskContext taskcontext, SakerPath filepath, SakerFile file, SakerDirectory outdir,
			SakerPath workingdirpath, ConverterContext cc) {
		byte[] result;
		try {
			SakerPath relpath = workingdirpath.relativize(filepath);
			result = translateFile(file, cc);
			SakerFile outputfile = new ByteArraySakerFile(file.getName(), result);

			taskcontext.getTaskUtilities().resolveDirectoryAtRelativePathCreate(outdir, relpath.getParent())
					.add(outputfile);
			return outputfile;
		} catch (DontIncludeLevelException e) {
		}
		return null;
	}

	private byte[] translateFile(SakerFile file, ConverterContext cc) throws DontIncludeLevelException {
		Set<String> demonames = new HashSet<>();
		boolean leaderboard = false;
		String cmd = null;
		byte[] uuidbytes = null;
		try (UnsyncByteArrayOutputStream os = new UnsyncByteArrayOutputStream();
				UnsyncByteArrayOutputStream headeros = new UnsyncByteArrayOutputStream();
				UnsyncByteArrayOutputStream demoos = new UnsyncByteArrayOutputStream();
				UnsyncByteArrayOutputStream descriptionos = new UnsyncByteArrayOutputStream();
				UnsyncByteArrayOutputStream yamyamos = new UnsyncByteArrayOutputStream();) {

			ByteArrayRegion filebytes = file.getBytes();

			// VERSION
			IntegerType.INSTANCE.serialize(SAPPHIRE_RELEASE_VERSION, headeros);

			SakerFile duplicate = null;
			String title = null;
			int difficulty = 10;
			int category = CATEGORY_NONE;

			int democount = 0;
			int demorandom = 0;
			String demotitle = null;

			int playercount = 1;
			try (BufferedReader reader = new BufferedReader(
					new InputStreamReader(new UnsyncByteArrayInputStream(filebytes)))) {
				for (String line; (line = reader.readLine()) != null;) {
					if (line.length() == 0)
						continue;
					int idx = line.indexOf(' ');
					if (idx < 0) {
						cmd = line.charAt(0) + "";
					} else {
						cmd = line.substring(0, idx);
					}
					switch (cmd) {
						case "uuid": {
							String uuidstr = line.substring(5);
							uuidbytes = new byte[16];
							byte[] strbytes = uuidstr.getBytes();
							for (int i = 0; i < strbytes.length; i += 2) {
								byte b1 = strbytes[i];
								byte b2 = strbytes[i + 1];
								if (b1 >= 'a') {
									uuidbytes[i / 2] |= (b1 - 'a' + 10) << 4;
								} else {
									uuidbytes[i / 2] |= (b1 - '0') << 4;
								}
								if (b2 >= 'a') {
									uuidbytes[i / 2] |= (b2 - 'a' + 10);
								} else {
									uuidbytes[i / 2] |= (b2 - '0');
								}
							}
							break;
						}
						case "leaderboard": {
							String content = line.substring(12);

							leaderboard = true;
							headeros.write(SAPPHIRE_CMD_LEADERBOARDS);
							IntegerType.INSTANCE.serialize(leaderboardTypesToFlags(content), headeros);
							break;
						}
						case "i": {
							if (line.length() > 2) {
								String info = line.substring(2).trim();
								if (descriptionos.size() > 0) {
									if (!Character.isAlphabetic(info.codePointAt(0))) {
										descriptionos.write('\n');
									} else {
										descriptionos.write(' ');
									}
								}
								descriptionos.write(info.getBytes());
							}
							break;
						}
						case "Y": {// silent yamyam
							os.write(cmd.charAt(0));
							break;
						}
						case "L": {// silent laser and explosion
							os.write(cmd.charAt(0));
							break;
						}
						case "M": {
							String atnumber = line.substring(2);
							String musicname = atnumber.substring(atnumber.indexOf(' ') + 1);
							synchronized (cc.musicCounts) {
								Integer counter = cc.musicCounts.get(musicname);
								if (counter == null) {
									counter = 1;
								} else {
									counter++;
								}
								cc.musicCounts.put(musicname, counter);
							}

							os.write(',');
							byte[] musicbytes = musicname.getBytes();
							IntegerType.INSTANCE.serialize(musicbytes.length, os);
							os.write(musicbytes);
//							synchronized (musicIds) {
//								if (musicIds.containsKey(musicname)) {
//									os.write('M');
//									IntegerType.INSTANCE.serialize(musicIds.get(musicname), os);
//								}
//							}

							break;
						}
						case "n": { // title
							title = line.substring(2).trim();
							break;
						}
						case "a": { // author
							byte[] author = line.substring(2).getBytes();

							headeros.write(cmd.charAt(0));
							IntegerType.INSTANCE.serialize(author.length, headeros);
							headeros.write(author);
							// headeros.write(new byte[16]);// author UUID, all zero for built in levels
							break;
						}
						case "C": { // category
							String[] split = line.split(" ");
							category = Integer.parseInt(split[1]) - 1;

							break;
						}
						case "D": { // difficulty
							String[] split = line.split(" ");
							difficulty = Integer.parseInt(split[1]);
							break;
						}
						case "E": { // max loot lose
							String[] split = line.split(" ");
							if (split.length > 1) {
								os.write(cmd.charAt(0));
								IntegerType.INSTANCE.serialize(Integer.parseInt(split[1]), os);
							}
							break;
						}
						case "w": // wheel time
						case "o": // robot move rate
						case "p": // push probability
						case "s": // swamp rate
						case "d": // dispenser speed
						case "v": // elevator speed
						case "t": // max steps
						case "T": // max time (turns)
						{
							String[] split = line.split(" ");
							if (split.length > 1) {
								int count = Integer.parseInt(split[1]);
								os.write(cmd.charAt(0));
								IntegerType.INSTANCE.serialize(count, os);
							}
							break;
						}
						case "e": {// lootcount
							String[] split = line.split(" ");
							if (!split[1].equals("*")) {
								int lootcount = Integer.parseInt(split[1]);

								os.write(cmd.charAt(0));
								IntegerType.INSTANCE.serialize(lootcount, os);
							}
							break;
						}
						case "m": { // map
							String[] split = line.split(" ");
							int w = Integer.parseInt(split[1]);
							int h = Integer.parseInt(split[2]);
							try (UnsyncByteArrayOutputStream mapos = new UnsyncByteArrayOutputStream()) {
								mapos.write(cmd.charAt(0));
								IntegerType.INSTANCE.serialize(w, mapos);
								IntegerType.INSTANCE.serialize(h, mapos);
								String map = "";
								for (int i = 0; i < h && map.length() < w * h; i++) {
									// replace auxilary animations with stone walls
									String mline = reader.readLine().replaceAll("[ACDFHIJKLMNOPTUWXZef]", "+");

									if (mline.contains("2")) {
										// has player 2
										playercount = 2;
									}

									map += mline;
									byte[] mlinebytes = mline.getBytes();
									// if (mlinebytes.length != w)
									// throw new IOException(
									// "Invalid width " + mlinebytes.length + " for: " + w + " file: " + file + " with line: " + mline);

									mapos.write(mlinebytes);
								}
								if (map.length() != w * h) {
									throw new IOException(
											"Failed to convert map, dimensions doesnt equal map element count: "
													+ (w * h) + " != " + map.length());
								}
								synchronized (cc.convertedMaps) {
									duplicate = cc.convertedMaps.get(map);
									cc.convertedMaps.put(map, file);
								}
								mapos.writeTo((OutputStream) os);
							}
							break;
						}
						case "y": { // yamyam
							String remainder = line.substring(2).replaceAll("[ACDFHIJKLMNOPTUWXZef]", "+");
							while (remainder.length() < 9) {
								remainder += " ";
							}
							if (remainder.contains("2")) {
								playercount = 2;
							}
							yamyamos.write(remainder.getBytes());
							break;
						}
						case "R": {
							if (demotitle != null && !demotitle.equals("Suspended")) {
								if (!demonames.add(demotitle.toLowerCase())) {
									SakerLog.warning().path(file.getSakerPath())
											.println("Duplicate demo names for level " + title + " - " + demotitle);
								}
								++democount;
								if ((demoos.size() % playercount) != 0) {
									throw new RuntimeException("Demo length is incorrect: " + demoos.size()
											+ " for player count: " + playercount + " in file: " + file.getName());
								}
								writeDemo(os, demorandom, demotitle, demoos.toByteArray());
							}
							demoos.reset();
							demorandom = Integer.parseUnsignedInt(line.split(" ")[1].trim());
							break;
						}
						case "1": {
							demotitle = line.substring(2).trim();
							break;
						}
						case "2": {
							demoos.write(line.substring(2).getBytes());
							break;
						}
						default: {
							// System.out.println("Unread Sapphire Level property: " + file + ": " + line);
							break;
						}
					}
				}

			}
			if (demotitle != null && !demotitle.equals("Suspended")) {
				++democount;
				if ((demoos.size() % playercount) != 0) {
					throw new RuntimeException("Demo length is incorrect: " + demoos.size() + " for player count: "
							+ playercount + " in file: " + file.getName());
				}
				writeDemo(os, demorandom, demotitle, demoos.toByteArray());
			}
			writeYamYam(os, yamyamos.toByteArray());
			if (descriptionos.size() > 0) {
				os.write('i');
				IntegerType.INSTANCE.serialize(descriptionos.size(), os);
				descriptionos.writeTo((OutputStream) os);
			}

			if (title == null) {
				synchronized (this) {
					title = "Unnamed level " + cc.unnamedId++;
				}
			}
			synchronized (cc.levelNames) {
				SakerFile prev = cc.levelNames.putIfAbsent(title, file);
				if (prev != null) {
					throw new RuntimeException(
							"Level with name already exists: " + title + " as " + prev + " and " + file);
				}
			}

			if (difficulty < 0 || difficulty > 10) {
				// Difficulty Unrated
				difficulty = 10;
			}
			if (difficulty > maxDifficulty) {
				throw new DontIncludeLevelException();
			}
			if (category < 0 || category > CATEGORY_NONE) {
				category = CATEGORY_NONE;
			}
			if (uuidbytes == null) {
				try {
					MessageDigest digest = MessageDigest.getInstance("SHA-256");
					digest.update(filebytes.getArray(), filebytes.getOffset(), filebytes.getLength());
					uuidbytes = Arrays.copyOf(digest.digest(), 16);
				} catch (NoSuchAlgorithmException e) {
					throw new RuntimeException("Failed to get SHA-256 hash of level for UUID generation.", e);
				}
				SakerLog.warning().path(file.getSakerPath()).println(
						"No uuid found in: " + file + " generated one using SHA-256: " + uuidBytesToString(uuidbytes));
			}
			if (uuidbytes.length < 16) {
				SakerLog.warning().path(file.getSakerPath())
						.println("Level uuid is too short: " + uuidbytes.length + " < " + 16);
				byte[] nbytes = new byte[16];
				System.arraycopy(uuidbytes, 0, nbytes, 16 - uuidbytes.length, uuidbytes.length);
				uuidbytes = nbytes;
			} else if (uuidbytes.length > 16) {
				SakerLog.warning().path(file.getSakerPath())
						.println("Level uuid is too long: " + uuidbytes.length + " > " + 16);
				uuidbytes = Arrays.copyOf(uuidbytes, 16);
			}
			if (!leaderboard) {
				SakerLog.warning().path(file.getSakerPath()).println("No leaderboard data found for: " + file);
			}

			String uuidmapval = title + "\t" + DIFFICULTY_MAP[difficulty] + "\t" + CATEGORY_MAP[category];
			String uuidstring = uuidBytesToString(uuidbytes);
			synchronized (cc.uuidMap) {
				String prev = cc.uuidMap.putIfAbsent(uuidstring, uuidmapval);
				if (prev != null) {
					throw new IllegalArgumentException(
							"Duplicate maps with UUID: " + uuidstring + " as: " + uuidmapval + " and " + prev);
				}
			}

			os.write(SAPPHIRE_CMD_END_OF_FILE);

			// difficulty stay -1

			headeros.write('n');
			byte[] titlebytes = title.getBytes();
			IntegerType.INSTANCE.serialize(titlebytes.length, headeros);
			headeros.write(titlebytes);
			headeros.write('D');
			IntegerType.INSTANCE.serialize(difficulty, headeros);
			headeros.write('C');
			IntegerType.INSTANCE.serialize(category, headeros);
			headeros.write(SAPPHIRE_CMD_DEMOCOUNT);
			IntegerType.INSTANCE.serialize(democount, headeros);
			headeros.write(SAPPHIRE_CMD_PLAYERCOUNT);
			IntegerType.INSTANCE.serialize(playercount, headeros);

			headeros.write(SAPPHIRE_CMD_UUID);
			headeros.write(uuidbytes, uuidbytes.length - 16, 16);
			//non-modifyable flag
			headeros.write(SAPPHIRE_CMD_NON_MODIFYABLE_FLAG);

			byte[] osbytes = new byte[headeros.size() + os.size()];
			System.arraycopy(headeros.toByteArray(), 0, osbytes, 0, headeros.size());
			System.arraycopy(os.toByteArray(), 0, osbytes, headeros.size(), os.size());

			if (duplicate != null) {
				SakerLog.warning().path(file.getSakerPath()).println("Duplicate game map with: " + duplicate.getName());
			}

			return osbytes;
		} catch (IOException | NumberFormatException | ArrayIndexOutOfBoundsException e) {
			throw new RuntimeException("Error parsing file: " + file.getName(),
					e/* file.getPath(), "Failed to parse file with cmd: \'" + cmd + "\'" */);
		}
	}

	private static void writeDemo(OutputStream os, int randomseed, String title, byte[] data) throws IOException {
		os.write('R');
		IntegerType.INSTANCE.serialize(randomseed, os);
		byte[] titlebytes = title.getBytes();
		IntegerType.INSTANCE.serialize(titlebytes.length, os);
		os.write(titlebytes);
		IntegerType.INSTANCE.serialize(data.length, os);
		os.write(data);
	}

	private static void writeYamYam(OutputStream os, byte[] data) throws IOException {
		if (data.length == 0)
			return;
		if (data.length % 9 != 0)
			throw new IOException("Invalid yamyam remainder length: " + data.length);
		os.write('y');
		IntegerType.INSTANCE.serialize(data.length / 9, os);
		os.write(data);
	}

	private void parseMusicFile(SakerFile file, ConverterContext cc) {
		// remove extension
		String filename = file.getName();
		String name = filename;
		int underscoreindex = filename.indexOf('_');

		name = name.substring(underscoreindex + 1, name.length() - 4);
		String lowername = name.toLowerCase();
		if (cc.musicFiles.containsKey(lowername)) {
			throw new RuntimeException("Duplicate music file: " + cc.musicFiles.get(lowername) + " - " + file);
		}
		cc.musicFiles.put(lowername, file);
		cc.musicNames.put(lowername, name);
//		String idstr = filename.substring(0, underscoreindex);
//		int id = /* idstr.length() == 0 ? 0 : */Integer.parseInt(idstr);
	}

	private static int leaderboardTypesToFlags(String content) {
		int leaderboards = 0;
		if (content.contains("gems")) {
			leaderboards |= LEADERBOARD_GEMS;
		}
		if (content.contains("steps")) {
			leaderboards |= LEADERBOARD_STEPS;
		}
		if (content.contains("time")) {
			leaderboards |= LEADERBOARD_TIME;
		}
		return leaderboards;
	}

	private static class DontIncludeLevelException extends Exception {
		private static final long serialVersionUID = -6134567452252885524L;
	}

	private static String uuidBytesToString(byte[] bytes) {
		char[] chars = new char[bytes.length * 2];
		for (int i = 0; i < bytes.length; i++) {
			byte b1 = (byte) ((bytes[i] >>> 4) & 0x0F);
			byte b2 = (byte) (bytes[i] & 0x0F);
			if (b1 < 10) {
				chars[i * 2] = (char) ('0' + b1);
			} else {
				chars[i * 2] = (char) ('a' + (b1 - 10));
			}
			if (b2 < 10) {
				chars[i * 2 + 1] = (char) ('0' + b2);
			} else {
				chars[i * 2 + 1] = (char) ('a' + (b2 - 10));
			}
		}
		return new String(chars);
	}

	private static class ConverterContext {
		Map<String, String> uuidMap = new TreeMap<>();
		int warnedLeaderboards = 40;

		Map<String, SakerFile> musicFiles = new TreeMap<>();
		Map<String, String> musicNames = new TreeMap<>();
		Map<String, Integer> musicCounts = new TreeMap<>();

		Map<String, SakerFile> convertedMaps = new TreeMap<>();
		Map<String, SakerFile> levelNames = new TreeMap<>();

		int unnamedId = 1;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeInt(maxDifficulty);
		SerialUtils.writeExternalCollection(out, levelcollectionstrategies);
		SerialUtils.writeExternalCollection(out, musiccollectionstrategies);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		maxDifficulty = in.readInt();
		levelcollectionstrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
		musiccollectionstrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((levelcollectionstrategies == null) ? 0 : levelcollectionstrategies.hashCode());
		result = prime * result + maxDifficulty;
		result = prime * result + ((musiccollectionstrategies == null) ? 0 : musiccollectionstrategies.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		SapphireLevelConverterWorkerTaskFactory other = (SapphireLevelConverterWorkerTaskFactory) obj;
		if (levelcollectionstrategies == null) {
			if (other.levelcollectionstrategies != null)
				return false;
		} else if (!levelcollectionstrategies.equals(other.levelcollectionstrategies))
			return false;
		if (maxDifficulty != other.maxDifficulty)
			return false;
		if (musiccollectionstrategies == null) {
			if (other.musiccollectionstrategies != null)
				return false;
		} else if (!musiccollectionstrategies.equals(other.musiccollectionstrategies))
			return false;
		return true;
	}

}
