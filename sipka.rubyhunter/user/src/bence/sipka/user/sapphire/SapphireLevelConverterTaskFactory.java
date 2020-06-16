/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package bence.sipka.user.sapphire;

import java.io.BufferedReader;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.security.SecureRandom;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
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
import saker.build.file.path.WildcardPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;
import saker.nest.utils.FrontendTaskFactory;

public class SapphireLevelConverterTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.level.convert";

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

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, SakerPath> assets;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, SakerPath> assets) {
			this.assets = ImmutableUtils.makeImmutableNavigableMap(assets);
		}

		public NavigableMap<String, SakerPath> getAssets() {
			return assets;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			SerialUtils.writeExternalMap(out, assets);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			assets = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assets == null) ? 0 : assets.hashCode());
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
			Output other = (Output) obj;
			if (assets == null) {
				if (other.assets != null)
					return false;
			} else if (!assets.equals(other.assets))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = "MaxDifficulty")
			private int maxDifficulty = 10;

			@SakerInput(value = "Levels")
			private Collection<WildcardPath> sapphireLevelsOption = Collections.emptyList();
			@SakerInput(value = "Music")
			private Collection<WildcardPath> sapphireMusicOption = Collections.emptyList();

			private Map<String, String> uuidMap = new HashMap<>();
			private int warnedLeaderboards = 40;

			private TreeMap<String, SakerFile> musicFiles = new TreeMap<>();
			private TreeMap<String, String> musicNames = new TreeMap<>();
			private TreeMap<String, Integer> musicCounts = new TreeMap<>();
			private SecureRandom secureRandom = null;

			private Map<String, SakerFile> convertedMaps = new HashMap<>();
			private Map<String, SakerFile> levelNames = new HashMap<>();

			private int unnamedId = 1;

			private synchronized SecureRandom getSecureRandom() {
				if (secureRandom == null) {
					secureRandom = new SecureRandom();
				}
				return secureRandom;
			}

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				genDirectory.clear();

				Collection<FileCollectionStrategy> levelcollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : sapphireLevelsOption) {
					levelcollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				NavigableMap<SakerPath, SakerFile> levelinputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, levelcollectionstrategies);

				Collection<FileCollectionStrategy> musiccollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : sapphireMusicOption) {
					musiccollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				NavigableMap<SakerPath, SakerFile> musicinputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, musiccollectionstrategies);

				for (Entry<SakerPath, SakerFile> entry : musicinputfiles.entrySet()) {
					this.parseMusicFile(entry.getValue());
				}

				NavigableMap<String, SakerPath> assets = new TreeMap<>();

				SakerPath workingdirpath = taskcontext.getTaskWorkingDirectoryPath();

				for (Entry<SakerPath, SakerFile> entry : levelinputfiles.entrySet()) {
					SakerFile convertresult = convertFile(taskcontext, entry.getKey(), entry.getValue(), genDirectory,
							workingdirpath);
					if (convertresult != null) {
						assets.put(workingdirpath.relativize(entry.getKey()).toString(), convertresult.getSakerPath());
					}
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(genDirectory.getFilesRecursiveByPath(
								genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				genDirectory.synchronize();

				Output result = new Output(assets);
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}

			private SakerFile convertFile(TaskContext taskcontext, SakerPath filepath, SakerFile file,
					SakerDirectory outdir, SakerPath workingdirpath) {
				byte[] result;
				try {
					SakerPath relpath = workingdirpath.relativize(filepath);
					result = translateFile(file);
					SakerFile outputfile = new ByteArraySakerFile(file.getName(), result);

					taskcontext.getTaskUtilities().resolveDirectoryAtRelativePathCreate(outdir, relpath.getParent())
							.add(outputfile);
					return outputfile;
				} catch (DontIncludeLevelException e) {
				}
				return null;
			}

			private byte[] translateFile(SakerFile file) throws DontIncludeLevelException {
				Set<String> demonames = new HashSet<>();
				boolean leaderboard = false;
				String cmd = null;
				byte[] uuidbytes = null;
				try (UnsyncByteArrayOutputStream os = new UnsyncByteArrayOutputStream();
						UnsyncByteArrayOutputStream headeros = new UnsyncByteArrayOutputStream();
						UnsyncByteArrayOutputStream demoos = new UnsyncByteArrayOutputStream();
						UnsyncByteArrayOutputStream descriptionos = new UnsyncByteArrayOutputStream();
						UnsyncByteArrayOutputStream yamyamos = new UnsyncByteArrayOutputStream();) {

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
					try (BufferedReader reader = new BufferedReader(new InputStreamReader(file.openInputStream()))) {
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
									synchronized (musicCounts) {
										Integer counter = musicCounts.get(musicname);
										if (counter == null) {
											counter = 1;
										} else {
											counter++;
										}
										musicCounts.put(musicname, counter);
									}

									os.write(',');
									byte[] musicbytes = musicname.getBytes();
									IntegerType.INSTANCE.serialize(musicbytes.length, os);
									os.write(musicbytes);
//									synchronized (musicIds) {
//										if (musicIds.containsKey(musicname)) {
//											os.write('M');
//											IntegerType.INSTANCE.serialize(musicIds.get(musicname), os);
//										}
//									}

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
										synchronized (convertedMaps) {
											duplicate = convertedMaps.get(map);
											convertedMaps.put(map, file);
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
											SakerLog.warning().path(file.getSakerPath()).println(
													"Duplicate demo names for level " + title + " - " + demotitle);
										}
										++democount;
										if ((demoos.size() % playercount) != 0) {
											throw new RuntimeException(
													"Demo length is incorrect: " + demoos.size() + " for player count: "
															+ playercount + " in file: " + file.getName());
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
							throw new RuntimeException("Demo length is incorrect: " + demoos.size()
									+ " for player count: " + playercount + " in file: " + file.getName());
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
							title = "Unnamed level " + unnamedId++;
						}
					}
					synchronized (levelNames) {
						if (levelNames.containsKey(title)) {
							throw new RuntimeException("Level with name already exists: " + title + " as "
									+ levelNames.get(title) + " " + file);
						}
						levelNames.put(title, file);
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
						throw new IllegalArgumentException("Missing level uuid: " + file);
					}
					if (uuidbytes.length < 16) {
						byte[] nbytes = new byte[16];
						System.arraycopy(uuidbytes, 0, nbytes, 16 - uuidbytes.length, uuidbytes.length);
						uuidbytes = nbytes;
					}
					String uuidstring = uuidBytesToString(uuidbytes);
					if (!leaderboard) {
						SakerLog.error().path(file.getSakerPath()).println("No leaderboard data found for: " + file);
						throw new RuntimeException("No leaderboard data found for: " + file);
					}

					synchronized (uuidMap) {
						uuidMap.put(uuidstring,
								title + "\t" + DIFFICULTY_MAP[difficulty] + "\t" + CATEGORY_MAP[category]);
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
						SakerLog.warning().path(file.getSakerPath())
								.println("Duplicate game map with: " + duplicate.getName());
					}

					return osbytes;
				} catch (IOException | NumberFormatException | ArrayIndexOutOfBoundsException e) {
					throw new RuntimeException("Error parsing file: " + file.getName(),
							e/* file.getPath(), "Failed to parse file with cmd: \'" + cmd + "\'" */);
				}
			}

			private void writeDemo(OutputStream os, int randomseed, String title, byte[] data) throws IOException {
				os.write('R');
				IntegerType.INSTANCE.serialize(randomseed, os);
				byte[] titlebytes = title.getBytes();
				IntegerType.INSTANCE.serialize(titlebytes.length, os);
				os.write(titlebytes);
				IntegerType.INSTANCE.serialize(data.length, os);
				os.write(data);
			}

			private void writeYamYam(OutputStream os, byte[] data) throws IOException {
				if (data.length == 0)
					return;
				if (data.length % 9 != 0)
					throw new IOException("Invalid yamyam remainder length: " + data.length);
				os.write('y');
				IntegerType.INSTANCE.serialize(data.length / 9, os);
				os.write(data);
			}

			private void parseMusicFile(SakerFile file) {
				// remove extension
				String filename = file.getName();
				String name = filename;
				int underscoreindex = filename.indexOf('_');

				name = name.substring(underscoreindex + 1, name.length() - 4);
				String lowername = name.toLowerCase();
				if (musicFiles.containsKey(lowername)) {
					throw new RuntimeException("Duplicate music file: " + musicFiles.get(lowername) + " - " + file);
				}
				musicFiles.put(lowername, file);
				musicNames.put(lowername, name);
//				String idstr = filename.substring(0, underscoreindex);
//				int id = /* idstr.length() == 0 ? 0 : */Integer.parseInt(idstr);
			}

		};
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

}
