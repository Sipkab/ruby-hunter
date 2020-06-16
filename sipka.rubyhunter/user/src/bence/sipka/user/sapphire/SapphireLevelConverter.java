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
//package bence.sipka.user.sapphire;
//
//import java.io.BufferedReader;
//import java.io.ByteArrayOutputStream;
//import java.io.IOException;
//import java.io.InputStreamReader;
//import java.io.OutputStream;
//import java.nio.file.Files;
//import java.nio.file.Path;
//import java.nio.file.Paths;
//import java.security.SecureRandom;
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Collections;
//import java.util.HashMap;
//import java.util.HashSet;
//import java.util.Map;
//import java.util.Map.Entry;
//import java.util.Set;
//import java.util.TreeMap;
//
//import bence.sipka.compiler.CCompilerOptions;
//import bence.sipka.compiler.CCompilerPass;
//import bence.sipka.compiler.types.builtin.IntegerType;
//import bence.sipka.repository.FrameworkModule;
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularParameter;
//import modular.core.file.ByteArrayModularFile;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.file.content.FileContentDescriptor;
//import modular.core.file.content.HashContentDescriptor;
//import modular.core.file.content.SerializableContentDescriptor;
//import modular.core.file.path.WildcardPath;
//import modular.core.module.ModuleOperation;
//import modular.core.util.ThreadUtils;
//
//@ModuleURI(SapphireLevelConverter.MODULE_URI)
//public class SapphireLevelConverter extends FrameworkModule {
//	public static final String MODULE_URI = "bence.sipka.sapphire.levelconverter";
//
//	private static final byte SAPPHIRE_CMD_DEMOCOUNT = (byte) 128;
//	private static final byte SAPPHIRE_CMD_PLAYERCOUNT = (byte) 129;
//	private static final byte SAPPHIRE_CMD_UUID = (byte) 130;
//	private static final byte SAPPHIRE_CMD_LEADERBOARDS = (byte) 131;
//	private static final byte SAPPHIRE_CMD_NON_MODIFYABLE_FLAG = 'X';
//	private static final byte SAPPHIRE_CMD_END_OF_FILE = (byte) 0;
//
//	private static final int LEADERBOARD_GEMS = 0x01;
//	private static final int LEADERBOARD_TIME = 0x02;
//	private static final int LEADERBOARD_STEPS = 0x04;
//
//	private static final String[] CATEGORY_MAP = { null, "Fun", "Discovery", "Action", "Battle", "Puzzle", "Science", "Work", "None" };
//	private static final String[] DIFFICULTY_MAP = { "Tutorial", "Simple", "Easy", "Moderate", "Normal", "Tricky", "Tough", "Difficult", "Hard", "M.A.D.",
//			"Unrated" };
//	private static final int CATEGORY_NONE = 7;
//	public static final int SAPPHIRE_RELEASE_VERSION = 3;
//
//	private int warnedLeaderboards = 40;
//
//	private static class DontIncludeLevelException extends Exception {
//		private static final long serialVersionUID = -6134567452252885524L;
//	}
//
//	@Operation("convert")
//	public class ConvertOperation extends ModuleOperation {
//
//		private Collection<ModularFile> toConvert = new ArrayList<>();
//
//		private Map<String, ModularFile> convertedMaps = new HashMap<>();
//		private Map<String, ModularFile> levelNames = new HashMap<>();
//
//		private Map<String, String> uuidMap = new HashMap<>();
//
//		private int unnamedId = 1;
//
//		private TreeMap<String, ModularFile> musicFiles = new TreeMap<>();
//		private TreeMap<String, String> musicNames = new TreeMap<>();
//		private TreeMap<String, Integer> musicCounts = new TreeMap<>();
//
//		private SecureRandom secureRandom = null;
//
//		@ModularParameter
//		private int maxDifficulty = 10;
//
//		@ModularParameter
//		private Collection<WildcardPath> SapphireLevels = Collections.emptyList();
//		@ModularParameter
//		private Collection<WildcardPath> SapphireMusic = Collections.emptyList();
//
//		@ModularData
//		private Collection<CCompilerOptions> CGlobalCompilerOptions = new ArrayList<>();
//		@ModularData
//		private Collection<CCompilerPass> CCompilerPasses = new ArrayList<>();
//
//		private synchronized SecureRandom getSecureRandom() {
//			if (secureRandom == null) {
//				secureRandom = new SecureRandom();
//			}
//			return secureRandom;
//		}
//
//		@Override
//		public void run() throws Exception {
//			ModularDirectory modulebuilddir = getModuleBuildModularDirectory();
////			ModularDirectory gendir = modulebuilddir.getDirectoryCreate("gen");
//
//			CCompilerOptions cppoptions = CCompilerOptions.create();
//			cppoptions.setLanguage("C++");
//			cppoptions.getIncludeDirectories().add(modulebuilddir.getModularPath().toString());
//			CGlobalCompilerOptions.add(cppoptions);
//
//			for (ModularFile file : WildcardPath.getFiles(getContext(), SapphireLevels)) {
//				toConvert.add(file);
//			}
//			for (WildcardPath fp : SapphireMusic) {
//				for (ModularFile file : fp.getFiles(getContext())) {
//					this.parseMusicFile(file);
//				}
//			}
//
//			ThreadUtils.runParallel(toConvert, this::convertFile);
//
////			musicCounts.entrySet().stream().sorted((l, r) -> Integer.compare(l.getValue(), r.getValue())).forEach(System.out::println);
////			for (Entry<String, Integer> entry : musicCounts.entrySet()) {
////				System.out.println("SapphireLevelConverter.ConvertOperation.run() " + entry);
////			}
//
////			Path workingdirpath = getContext().getWorkingDirectory();
////			Map<String, String> musicfiles = musicFiles.entrySet().stream()
////					.collect(Collectors.toMap(e -> e.getKey().replaceAll("[ ]+", "_"), e -> workingdirpath.relativize(e.getValue().getPath()).toString()
////							.replaceAll("[\\\\/]", "::").replaceAll("[\\. ]+", "_").replaceAll("::([0-9])", "::_$1"), (u, v) -> {
////								throw new IllegalStateException(String.format("Duplicate key %s", u));
////							}, TreeMap::new));
////			Map<String, String> musicnames = musicNames.entrySet().stream()
////					.collect(Collectors.toMap(e -> e.getKey().replaceAll("[ ]+", "_"), e -> e.getValue(), (u, v) -> {
////						throw new IllegalStateException(String.format("Duplicate key %s", u));
////					}, TreeMap::new));
////
////			gendir.add(new TemplatedSourceModularFile("sapphiremusic.h",
////					new TemplatedSource(descriptor::getInputStream, "gen/sapphiremusic.h").setVar("musicids", musicIds).setVar("musicfiles", musicfiles)));
////
////			ModularFile cppfile = new TemplatedSourceModularFile("sapphiremusic.cpp",
////					new TemplatedSource(descriptor::getInputStream, "gen/sapphiremusic.cpp").setVar("musicnames", new ArrayList<>(musicnames.values())));
////			gendir.add(cppfile);
////
////			CCompilerPass cpppass = CCompilerPass.create();
////			cpppass.setLanguage("C++");
////			cpppass.getFiles().add(cppfile.getPath().toString());
////			CCompilerPasses.add(cpppass);
//
//			modulebuilddir.add(new ModularFile("level_info.txt") {
//				@Override
//				public void writeToStream(OutputStream os) throws IOException {
//					for (Entry<String, String> e : uuidMap.entrySet()) {
//						os.write(e.getKey().getBytes());
//						os.write('\t');
//						os.write(e.getValue().getBytes());
//						os.write('\n');
//					}
//				}
//
//				@Override
//				public FileContentDescriptor getContentDescriptor() {
//					return new SerializableContentDescriptor(uuidMap);
//				}
//			});
//
//		}
//
//		private void parseMusicFile(ModularFile file) {
//			// remove extension
//			String filename = file.getName();
//			String name = filename;
//			int underscoreindex = filename.indexOf('_');
//
//			name = name.substring(underscoreindex + 1, name.length() - 4);
//			String lowername = name.toLowerCase();
//			if (musicFiles.containsKey(lowername)) {
//				throw new RuntimeException("Duplicate music file: " + musicFiles.get(lowername) + " - " + file);
//			}
//			musicFiles.put(lowername, file);
//			musicNames.put(lowername, name);
////			String idstr = filename.substring(0, underscoreindex);
////			int id = /* idstr.length() == 0 ? 0 : */Integer.parseInt(idstr);
//		}
//
//		private void writeDemo(OutputStream os, int randomseed, String title, byte[] data) throws IOException {
//			os.write('R');
//			IntegerType.INSTANCE.serialize(randomseed, os);
//			byte[] titlebytes = title.getBytes();
//			IntegerType.INSTANCE.serialize(titlebytes.length, os);
//			os.write(titlebytes);
//			IntegerType.INSTANCE.serialize(data.length, os);
//			os.write(data);
//		}
//
//		private void writeYamYam(OutputStream os, byte[] data) throws IOException {
//			if (data.length == 0)
//				return;
//			if (data.length % 9 != 0)
//				throw new IOException("Invalid yamyam remainder length: " + data.length);
//			os.write('y');
//			IntegerType.INSTANCE.serialize(data.length / 9, os);
//			os.write(data);
//		}
//
//		private byte[] translateFile(ModularFile file) throws DontIncludeLevelException {
//			Set<String> demonames = new HashSet<>();
//			boolean leaderboard = false;
//			String cmd = null;
//			byte[] uuidbytes = null;
//			try (ByteArrayOutputStream os = new ByteArrayOutputStream();
//					ByteArrayOutputStream headeros = new ByteArrayOutputStream();
//					ByteArrayOutputStream demoos = new ByteArrayOutputStream();
//					ByteArrayOutputStream descriptionos = new ByteArrayOutputStream();
//					ByteArrayOutputStream yamyamos = new ByteArrayOutputStream();) {
//
//				// VERSION
//				IntegerType.INSTANCE.serialize(SAPPHIRE_RELEASE_VERSION, headeros);
//
//				ModularFile duplicate = null;
//				String title = null;
//				int difficulty = 10;
//				int category = CATEGORY_NONE;
//
//				int democount = 0;
//				int demorandom = 0;
//				String demotitle = null;
//
//				int playercount = 1;
//				try (BufferedReader reader = new BufferedReader(new InputStreamReader(file.openInputStream()))) {
//					for (String line; (line = reader.readLine()) != null;) {
//						if (line.length() == 0)
//							continue;
//						int idx = line.indexOf(' ');
//						if (idx < 0) {
//							cmd = line.charAt(0) + "";
//						} else {
//							cmd = line.substring(0, idx);
//						}
//						switch (cmd) {
//							case "uuid": {
//								String uuidstr = line.substring(5);
//								uuidbytes = new byte[16];
//								byte[] strbytes = uuidstr.getBytes();
//								for (int i = 0; i < strbytes.length; i += 2) {
//									byte b1 = strbytes[i];
//									byte b2 = strbytes[i + 1];
//									if (b1 >= 'a') {
//										uuidbytes[i / 2] |= (b1 - 'a' + 10) << 4;
//									} else {
//										uuidbytes[i / 2] |= (b1 - '0') << 4;
//									}
//									if (b2 >= 'a') {
//										uuidbytes[i / 2] |= (b2 - 'a' + 10);
//									} else {
//										uuidbytes[i / 2] |= (b2 - '0');
//									}
//								}
//								break;
//							}
//							case "leaderboard": {
//								String content = line.substring(12);
//
//								leaderboard = true;
//								headeros.write(SAPPHIRE_CMD_LEADERBOARDS);
//								IntegerType.INSTANCE.serialize(leaderboardTypesToFlags(content), headeros);
//								break;
//							}
//							case "i": {
//								if (line.length() > 2) {
//									String info = line.substring(2).trim();
//									if (descriptionos.size() > 0) {
//										if (!Character.isAlphabetic(info.codePointAt(0))) {
//											descriptionos.write('\n');
//										} else {
//											descriptionos.write(' ');
//										}
//									}
//									descriptionos.write(info.getBytes());
//								}
//								break;
//							}
//							case "Y": {// silent yamyam
//								os.write(cmd.charAt(0));
//								break;
//							}
//							case "L": {// silent laser and explosion
//								os.write(cmd.charAt(0));
//								break;
//							}
//							case "M": {
//								String atnumber = line.substring(2);
//								String musicname = atnumber.substring(atnumber.indexOf(' ') + 1);
//								synchronized (musicCounts) {
//									Integer counter = musicCounts.get(musicname);
//									if (counter == null) {
//										counter = 1;
//									} else {
//										counter++;
//									}
//									musicCounts.put(musicname, counter);
//								}
//
//								os.write(',');
//								byte[] musicbytes = musicname.getBytes();
//								IntegerType.INSTANCE.serialize(musicbytes.length, os);
//								os.write(musicbytes);
////								synchronized (musicIds) {
////									if (musicIds.containsKey(musicname)) {
////										os.write('M');
////										IntegerType.INSTANCE.serialize(musicIds.get(musicname), os);
////									}
////								}
//
//								break;
//							}
//							case "n": { // title
//								title = line.substring(2).trim();
//								break;
//							}
//							case "a": { // author
//								byte[] author = line.substring(2).getBytes();
//
//								headeros.write(cmd.charAt(0));
//								IntegerType.INSTANCE.serialize(author.length, headeros);
//								headeros.write(author);
//								// headeros.write(new byte[16]);// author UUID, all zero for built in levels
//								break;
//							}
//							case "C": { // category
//								String[] split = line.split(" ");
//								category = Integer.parseInt(split[1]) - 1;
//
//								break;
//							}
//							case "D": { // difficulty
//								String[] split = line.split(" ");
//								difficulty = Integer.parseInt(split[1]);
//								break;
//							}
//							case "E": { // max loot lose
//								String[] split = line.split(" ");
//								if (split.length > 1) {
//									os.write(cmd.charAt(0));
//									IntegerType.INSTANCE.serialize(Integer.parseInt(split[1]), os);
//								}
//								break;
//							}
//							case "w": // wheel time
//							case "o": // robot move rate
//							case "p": // push probability
//							case "s": // swamp rate
//							case "d": // dispenser speed
//							case "v": // elevator speed
//							case "t": // max steps
//							case "T": // max time (turns)
//							{
//								String[] split = line.split(" ");
//								if (split.length > 1) {
//									int count = Integer.parseInt(split[1]);
//									os.write(cmd.charAt(0));
//									IntegerType.INSTANCE.serialize(count, os);
//								}
//								break;
//							}
//							case "e": {// lootcount
//								String[] split = line.split(" ");
//								if (!split[1].equals("*")) {
//									int lootcount = Integer.parseInt(split[1]);
//
//									os.write(cmd.charAt(0));
//									IntegerType.INSTANCE.serialize(lootcount, os);
//								}
//								break;
//							}
//							case "m": { // map
//								String[] split = line.split(" ");
//								int w = Integer.parseInt(split[1]);
//								int h = Integer.parseInt(split[2]);
//								try (ByteArrayOutputStream mapos = new ByteArrayOutputStream()) {
//									mapos.write(cmd.charAt(0));
//									IntegerType.INSTANCE.serialize(w, mapos);
//									IntegerType.INSTANCE.serialize(h, mapos);
//									String map = "";
//									for (int i = 0; i < h && map.length() < w * h; i++) {
//										// replace auxilary animations with stone walls
//										String mline = reader.readLine().replaceAll("[ACDFHIJKLMNOPTUWXZef]", "+");
//
//										if (mline.contains("2")) {
//											// has player 2
//											playercount = 2;
//										}
//
//										map += mline;
//										byte[] mlinebytes = mline.getBytes();
//										// if (mlinebytes.length != w)
//										// throw new IOException(
//										// "Invalid width " + mlinebytes.length + " for: " + w + " file: " + file + " with line: " + mline);
//
//										mapos.write(mlinebytes);
//									}
//									if (map.length() != w * h) {
//										throw new IOException(
//												"Failed to convert map, dimensions doesnt equal map element count: " + (w * h) + " != " + map.length());
//									}
//									synchronized (convertedMaps) {
//										duplicate = convertedMaps.get(map);
//										convertedMaps.put(map, file);
//									}
//									mapos.writeTo(os);
//								}
//								break;
//							}
//							case "y": { // yamyam
//								String remainder = line.substring(2).replaceAll("[ACDFHIJKLMNOPTUWXZef]", "+");
//								while (remainder.length() < 9) {
//									remainder += " ";
//								}
//								if (remainder.contains("2")) {
//									playercount = 2;
//								}
//								yamyamos.write(remainder.getBytes());
//								break;
//							}
//							case "R": {
//								if (demotitle != null && !demotitle.equals("Suspended")) {
//									if (!demonames.add(demotitle.toLowerCase())) {
//										logWarning().path(file.getModularPath()).println("Duplicate demo names for level " + title + " - " + demotitle);
//									}
//									++democount;
//									if ((demoos.size() % playercount) != 0) {
//										throw new RuntimeException("Demo length is incorrect: " + demoos.size() + " for player count: " + playercount
//												+ " in file: " + file.getName());
//									}
//									writeDemo(os, demorandom, demotitle, demoos.toByteArray());
//								}
//								demoos.reset();
//								demorandom = Integer.parseUnsignedInt(line.split(" ")[1].trim());
//								break;
//							}
//							case "1": {
//								demotitle = line.substring(2).trim();
//								break;
//							}
//							case "2": {
//								demoos.write(line.substring(2).getBytes());
//								break;
//							}
//							default: {
//								// System.out.println("Unread Sapphire Level property: " + file + ": " + line);
//								break;
//							}
//						}
//					}
//
//				}
//				if (demotitle != null && !demotitle.equals("Suspended")) {
//					++democount;
//					if ((demoos.size() % playercount) != 0) {
//						throw new RuntimeException(
//								"Demo length is incorrect: " + demoos.size() + " for player count: " + playercount + " in file: " + file.getName());
//					}
//					writeDemo(os, demorandom, demotitle, demoos.toByteArray());
//				}
//				writeYamYam(os, yamyamos.toByteArray());
//				if (descriptionos.size() > 0) {
//					os.write('i');
//					IntegerType.INSTANCE.serialize(descriptionos.size(), os);
//					descriptionos.writeTo(os);
//				}
//
//				if (title == null) {
//					synchronized (this) {
//						title = "Unnamed level " + unnamedId++;
//					}
//				}
//				synchronized (levelNames) {
//					if (levelNames.containsKey(title)) {
//						throw new RuntimeException("Level with name already exists: " + title + " as " + levelNames.get(title) + " " + file);
//					}
//					levelNames.put(title, file);
//				}
//
//				if (difficulty < 0 || difficulty > 10) {
//					// Difficulty Unrated
//					difficulty = 10;
//				}
//				if (difficulty > maxDifficulty) {
//					throw new DontIncludeLevelException();
//				}
//				if (category < 0 || category > CATEGORY_NONE) {
//					category = CATEGORY_NONE;
//				}
//				if (uuidbytes == null) {
//					byte[] uuidarray = new byte[16];
//					uuidbytes = uuidarray;
//					getSecureRandom().nextBytes(uuidarray);
//					byte[] filecontent = file.getBytes();
//					ModularFile replacement = new ModularFile(file.getName()) {
//						@Override
//						public void writeToStream(OutputStream os) throws IOException {
//							os.write(filecontent);
//							os.write("\r\nuuid ".getBytes());
//							for (int i = 0; i < uuidarray.length; i++) {
//								String tostr = Integer.toUnsignedString(uuidarray[i] & 0xFF, 16);
//								while (tostr.length() < 2) {
//									tostr = "0" + tostr;
//								}
//								os.write(tostr.getBytes());
//							}
//							os.write('\r');
//							os.write('\n');
//						}
//
//						@Override
//						public FileContentDescriptor getContentDescriptor() {
//							return new HashContentDescriptor(this);
//						}
//					};
//					file.getParent().add(replacement);
//					replacement.synchronize();
//					file = replacement;
//				}
//				if (uuidbytes.length < 16) {
//					byte[] nbytes = new byte[16];
//					System.arraycopy(uuidbytes, 0, nbytes, 16 - uuidbytes.length, uuidbytes.length);
//					uuidbytes = nbytes;
//				}
//				String uuidstring = uuidBytesToString(uuidbytes);
//				if (!leaderboard) {
////					throw new RuntimeException("No leaderboard data found for: " + file);
//					try {
//						Path path = Paths.get("c:\\Users\\sipka\\AppData\\Local\\Ruby Hunter\\leaderboards", uuidstring);
//						if (Files.exists(path)) {
//							byte[] filecontent = file.getBytes();
//							byte[] leaderboardbytes = Files.readAllBytes(path);
//							headeros.write(SAPPHIRE_CMD_LEADERBOARDS);
//							IntegerType.INSTANCE.serialize(leaderboardTypesToFlags(new String(leaderboardbytes)), headeros);
//							ModularFile replacement = new ModularFile(file.getName()) {
//								@Override
//								public void writeToStream(OutputStream os) throws IOException {
//									os.write(filecontent);
//									os.write("\r\nleaderboard ".getBytes());
//									os.write(leaderboardbytes);
//									os.write('\r');
//									os.write('\n');
//								}
//
//								@Override
//								public FileContentDescriptor getContentDescriptor() {
//									return new HashContentDescriptor(this);
//								}
//							};
//							file.getParent().add(replacement);
//							replacement.synchronize();
//							file = replacement;
//						} else {
//							if (warnedLeaderboards-- > 0) {
//								logWarning().path(file.getModularPath()).println("No leaderboard found for map: " + title);
//							}
//						}
//					} catch (IOException e) {
//
//					}
//				}
//
//				synchronized (uuidMap) {
//					uuidMap.put(uuidstring, title + "\t" + DIFFICULTY_MAP[difficulty] + "\t" + CATEGORY_MAP[category]);
//				}
//
//				os.write(SAPPHIRE_CMD_END_OF_FILE);
//
//				// difficulty stay -1
//
//				headeros.write('n');
//				byte[] titlebytes = title.getBytes();
//				IntegerType.INSTANCE.serialize(titlebytes.length, headeros);
//				headeros.write(titlebytes);
//				headeros.write('D');
//				IntegerType.INSTANCE.serialize(difficulty, headeros);
//				headeros.write('C');
//				IntegerType.INSTANCE.serialize(category, headeros);
//				headeros.write(SAPPHIRE_CMD_DEMOCOUNT);
//				IntegerType.INSTANCE.serialize(democount, headeros);
//				headeros.write(SAPPHIRE_CMD_PLAYERCOUNT);
//				IntegerType.INSTANCE.serialize(playercount, headeros);
//
//				headeros.write(SAPPHIRE_CMD_UUID);
//				headeros.write(uuidbytes, uuidbytes.length - 16, 16);
//				//non-modifyable flag
//				headeros.write(SAPPHIRE_CMD_NON_MODIFYABLE_FLAG);
//
//				byte[] osbytes = new byte[headeros.size() + os.size()];
//				System.arraycopy(headeros.toByteArray(), 0, osbytes, 0, headeros.size());
//				System.arraycopy(os.toByteArray(), 0, osbytes, headeros.size(), os.size());
//
//				if (duplicate != null) {
//					logWarning().path(file.getModularPath()).println("Duplicate game map with: " + duplicate.getName());
//				}
//
//				return osbytes;
//			} catch (IOException | NumberFormatException | ArrayIndexOutOfBoundsException e) {
//				throw new RuntimeException("Error parsing file: " + file.getName(),
//						e/* file.getPath(), "Failed to parse file with cmd: \'" + cmd + "\'" */);
//			}
//		}
//
//		private void convertFile(ModularFile file) {
//			byte[] result;
//			ModularDirectory parent = file.getParent();
//			try {
//				result = translateFile(file);
//				parent.add(new ByteArrayModularFile(file.getName(), result), ModularFile.FLAG_SHADOWFILE);
//			} catch (DontIncludeLevelException e) {
//				parent.hide(file);
//			}
//		}
//
//		private String uuidBytesToString(byte[] bytes) {
//			char[] chars = new char[bytes.length * 2];
//			for (int i = 0; i < bytes.length; i++) {
//				byte b1 = (byte) ((bytes[i] >>> 4) & 0x0F);
//				byte b2 = (byte) (bytes[i] & 0x0F);
//				if (b1 < 10) {
//					chars[i * 2] = (char) ('0' + b1);
//				} else {
//					chars[i * 2] = (char) ('a' + (b1 - 10));
//				}
//				if (b2 < 10) {
//					chars[i * 2 + 1] = (char) ('0' + b2);
//				} else {
//					chars[i * 2 + 1] = (char) ('a' + (b2 - 10));
//				}
//			}
//			return new String(chars);
//		}
//	}
//
//	private int leaderboardTypesToFlags(String content) {
//		int leaderboards = 0;
//		if (content.contains("gems")) {
//			leaderboards |= LEADERBOARD_GEMS;
//		}
//		if (content.contains("steps")) {
//			leaderboards |= LEADERBOARD_STEPS;
//		}
//		if (content.contains("time")) {
//			leaderboards |= LEADERBOARD_TIME;
//		}
//		return leaderboards;
//	}
//}
