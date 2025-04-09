package bence.sipka.user.obj3d;

import java.io.BufferedReader;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Optional;
import java.util.Set;
import java.util.TreeMap;
import java.util.regex.Pattern;

import bence.sipka.user.obj3d.ObjectCollection.DuplicateObjectData;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class ObjectTranslatorWorkerTaskFactory implements TaskFactory<ObjectTranslatorTaskFactory.Output>,
		Task<ObjectTranslatorTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final String FILE_NAME_OBJECTS_3D_COLLECTION = "objects_3d_collection";

	private static final Pattern SPLIT_PATTERN = Pattern.compile("[\\s]+");
	private static final Pattern FACE_SPLIT_PATTERN = Pattern.compile("/");

	private Set<FileCollectionStrategy> inputCollectionStrategies;
	private MaterialTranslatorTaskFactory.Output materialsOption;

	/**
	 * For {@link Externalizable}.
	 */
	public ObjectTranslatorWorkerTaskFactory() {
	}

	public ObjectTranslatorWorkerTaskFactory(Set<FileCollectionStrategy> collectionstrategies,
			bence.sipka.user.obj3d.MaterialTranslatorTaskFactory.Output materialsOption) {
		this.inputCollectionStrategies = collectionstrategies;
		this.materialsOption = materialsOption;
	}

	@Override
	public Task<? extends ObjectTranslatorTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public ObjectTranslatorTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(ObjectTranslatorTaskFactory.TASK_NAME);

		Map<String, MaterialLibrary> materials = materialsOption.getMaterials();

		SakerPath gendirectorypath = taskcontext.getTaskBuildDirectoryPath().resolve(ObjectLinkerTaskFactory.TASK_NAME);
		NavigableMap<String, SakerPath> assets = new TreeMap<>();

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputCollectionStrategies);

		NavigableMap<SakerPath, SakerFile> toconvert = new TreeMap<>();
		NavigableMap<SakerPath, SakerFile> duplicates = new TreeMap<>();
		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			SakerFile file = entry.getValue();
			if (file.getName().endsWith(".dup.obj")) {
				duplicates.put(entry.getKey(), file);
			} else {
				toconvert.put(entry.getKey(), file);
			}
		}

		int objid = 0;

		ObjectCollection objcoll = new ObjectCollection();

		SakerPath taskworkingdirectorypath = taskcontext.getTaskWorkingDirectoryPath();

		for (Entry<SakerPath, SakerFile> entry : toconvert.entrySet()) {
			SakerFile f = entry.getValue();

			Material currentmat = null;
			SakerPath path = entry.getKey();
			ObjectData objdata = new ObjectData(objid++, path);
			MaterialLibrary matlib = null;
			SakerPath parentpath = path.getParent();
			try (BufferedReader br = new BufferedReader(new InputStreamReader(f.openInputStream()))) {
				int linenumber = 0;
				for (String line; (line = br.readLine()) != null; linenumber++) {
					line = line.trim();
					if (line.startsWith("#") || line.length() == 0) {
						continue;
					}
					String[] split = SPLIT_PATTERN.split(line);
					if (split.length == 0) {
						continue;
					}

					switch (split[0]) {
						case "mtllib": {
							String mtlpath = split[1];
							if (matlib != null) {
								throw new RuntimeException(
										"More than one material library: " + mtlpath + " " + path + ":" + linenumber);
							}
							SakerPath matlibpath = parentpath
									.resolve(mtlpath.substring(0, mtlpath.length() - 4) + ".mtl");// "remove .mtl"
							String matlibpathstring = matlibpath.toString().replace('\\', '/');

							matlib = materials.get(matlibpathstring);
							if (matlib == null) {
								throw new RuntimeException("Material library not found: " + matlibpathstring + " in "
										+ materials + " " + path + ":" + linenumber);
							}
							break;
						}
						case "usemtl": {
							currentmat = matlib.get(split[1]);
							break;
						}
						case "origin": {
							objdata.origin = new Vector(1, split);
							break;
						}
						case "f": {
							if (currentmat == null) {
								throw new RuntimeException("No current material specified " + path + ":" + linenumber);
							}
							Face face = new Face(currentmat);
							for (int i = 1; i < split.length; i++) {
								String[] fsplit = FACE_SPLIT_PATTERN.split(split[i]);
								VertexIndex v = new VertexIndex();
								v.posIndex = Integer.parseUnsignedInt(fsplit[0]) - 1;
								if (fsplit.length > 1) {
									if (fsplit[1].length() > 0) {
										v.textureIndex = Integer.parseUnsignedInt(fsplit[1]) - 1;
									}
									if (fsplit.length > 2) {
										if (fsplit[2].length() > 0) {
											v.normalIndex = Integer.parseUnsignedInt(fsplit[2]) - 1;
										}
									}
								}
								face.vertices.add(v);
							}
							if (face.vertices.size() != 3) {
								throw new RuntimeException("Face not triangle " + path + ":" + linenumber);
							}
							objdata.faces.add(face);
							break;
						}
						case "v": {
							objdata.vertices.add(new Vector(1, split));
							break;
						}
						case "vn": {
							Vector n = new Vector(1, split);
							n.w = 0.0f;
							objdata.normals.add(n);
							break;
						}
						case "vt": {
							objdata.textcoords.add(new Vector(1, split));
							break;
						}
						default: {
							// System.out.println(path + ":" + (linenumber + 1) + ": Warning: Ignored object directive: " + split[0]);
							break;
						}
					}
				}
			}
			objcoll.objects.add(objdata);
			String meshfilename = f.getName() + ".mesh";

//			assets.put(SakerPath.valueOf(ObjectLinkerTaskFactory.TASK_NAME + "/" + meshfilename).toString(),
//					gendirectorypath.resolve(meshfilename));
			assets.put(taskworkingdirectorypath.relativize(path).getParent().resolve(meshfilename).toString(),
					gendirectorypath.resolve(meshfilename));

//			genDirectory.add(new ObjectModularFile(meshfilename, objcoll, objdata));
		}

		for (Entry<SakerPath, SakerFile> entry : duplicates.entrySet()) {
			SakerFile f = entry.getValue();

			// ThreadUtils.runParallel(f -> {
			String[] content = f.getContent().split(" +");
			SakerFile realfile = taskcontext.getTaskUtilities().resolveFileAtPath(f.getParent(),
					SakerPath.valueOf(content[0]));
			if (realfile == null) {
				SakerLog.error().path(entry.getKey()).println("File not found: " + content[0]);
				throw new RuntimeException("File not found: " + content[0]);
				// realfile = operator.getProjectManager().getRootDirectory().findAllFile(Paths.get(content[0]));
			}
			SakerPath rfpath = realfile.getSakerPath();
			Optional<ObjectData> got = objcoll.objects.stream().filter(o -> rfpath.equals(o.path)).findAny();
			if (!got.isPresent()) {
				throw new RuntimeException("Objectdata not found. " + content[0]);
			}
			ObjectData objdata = got.get();
			MaterialLibrary matlib = materials
					.get(taskworkingdirectorypath.resolve(content[1]).toString().replace('\\', '/'));
			if (matlib == null) {
				throw new RuntimeException(
						"Material library not found with path: " + content[1] + " in map: " + materials);
			}
			String objmeshfilename = f.getName().substring(0, f.getName().length() - 7) + "obj.mesh";

//			assets.put(SakerPath.valueOf(ObjectLinkerTaskFactory.TASK_NAME + "/" + objmeshfilename).toString(),
//					gendirectorypath.resolve(objmeshfilename));
			assets.put(
					taskworkingdirectorypath.relativize(entry.getKey()).getParent().resolve(objmeshfilename).toString(),
					gendirectorypath.resolve(objmeshfilename));

			objcoll.duplicateDatas.add(new DuplicateObjectData(objmeshfilename, objdata, matlib));

//			genDirectory.add(new DuplicatObjectModularFile(objmeshfilename, objcoll, objdata, matlib));
			objcoll.colorMaterials.addAll(matlib.getColoreds());
			objcoll.textureMaterials.addAll(matlib.getTextureds());
		}

		assets.put("sapphire_yours/" + FILE_NAME_OBJECTS_3D_COLLECTION,
				gendirectorypath.resolve(FILE_NAME_OBJECTS_3D_COLLECTION));

		return new ObjectTranslatorTaskFactory.Output(objcoll, assets);
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, inputCollectionStrategies);
		out.writeObject(materialsOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		inputCollectionStrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
		materialsOption = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((inputCollectionStrategies == null) ? 0 : inputCollectionStrategies.hashCode());
		result = prime * result + ((materialsOption == null) ? 0 : materialsOption.hashCode());
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
		ObjectTranslatorWorkerTaskFactory other = (ObjectTranslatorWorkerTaskFactory) obj;
		if (inputCollectionStrategies == null) {
			if (other.inputCollectionStrategies != null)
				return false;
		} else if (!inputCollectionStrategies.equals(other.inputCollectionStrategies))
			return false;
		if (materialsOption == null) {
			if (other.materialsOption != null)
				return false;
		} else if (!materialsOption.equals(other.materialsOption))
			return false;
		return true;
	}

}
