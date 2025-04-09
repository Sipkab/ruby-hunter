package bence.sipka.user.obj3d;

import java.io.BufferedReader;
import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;
import java.util.regex.Pattern;

import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.runtime.execution.SakerLog;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class MaterialTranslatorWorkerTaskFactory implements TaskFactory<MaterialTranslatorTaskFactory.Output>,
		Task<MaterialTranslatorTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private static final Pattern SPLIT_PATTERN = Pattern.compile("[\\s]+");

	private Set<FileCollectionStrategy> inputCollectionStrategies;
	private List<List<Object>> diffuseToAmbientFactors;

	/**
	 * For {@link Externalizable}.
	 */
	public MaterialTranslatorWorkerTaskFactory() {
	}

	public MaterialTranslatorWorkerTaskFactory(Set<FileCollectionStrategy> collectionstrategies,
			List<List<Object>> diffuseToAmbientFactors) {
		this.inputCollectionStrategies = collectionstrategies;
		this.diffuseToAmbientFactors = diffuseToAmbientFactors;
	}

	@Override
	public Task<? extends MaterialTranslatorTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public MaterialTranslatorTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(MaterialTranslatorTaskFactory.TASK_NAME);

		ArrayList<List<Object>> factors = new ArrayList<>(diffuseToAmbientFactors);

		for (ListIterator<List<Object>> it = factors.listIterator(); it.hasNext();) {
			List<Object> l = ObjectUtils.newArrayList(it.next());
			l.set(0, Pattern.compile(l.get(0).toString()));
			it.set(l);
		}

		NavigableMap<String, MaterialLibrary> materials = new TreeMap<>();

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputCollectionStrategies);

		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			SakerFile f = entry.getValue();

			String ambientfactor = null;// f.getAttribute("diffuse_to_ambient_factor");
			for (int i = factors.size() - 1; i >= 0; i--) {
				List<Object> difentry = factors.get(i);
				if (((Pattern) difentry.get(0)).matcher(f.getName()).matches()) {
					ambientfactor = difentry.get(1).toString();
					break;
				}
			}

			MaterialLibrary lib = new MaterialLibrary();
			Material currentmat = null;
			SakerPath path = entry.getKey();
			SakerDirectory parent = f.getParent();
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
						case "newmtl": {
							currentmat = new Material(split[1]);
							lib.materials.add(currentmat);
							break;
						}
						case "Kd": {
							currentmat.diffuseColor = new Vector(1, split);
							if (ambientfactor != null) {
								float factor = Float.parseFloat(ambientfactor);
								currentmat.ambientColor = new Vector(currentmat.diffuseColor.x * factor,
										currentmat.diffuseColor.y * factor, currentmat.diffuseColor.z * factor);
							}
							break;
						}
						case "d": {
							currentmat.transparency = Float.parseFloat(split[1]);
							break;
						}
						case "Ka": {
							if (ambientfactor == null) {
								currentmat.ambientColor = new Vector(1, split);
							}
							break;
						}
						case "Ks": {
							currentmat.specularColor = new Vector(1, split);
							break;
						}
						case "Ns": {
							currentmat.specularExponent = Float.parseFloat(split[1]);
							break;
						}
						case "map_Kd": {
							SakerFile res = taskcontext.getTaskUtilities().resolveFileAtPath(parent,
									SakerPath.valueOf(split[1]));
							if (res == null) {
								SakerLog.error().path(f.getSakerPath()).line(linenumber)
										.println("Material texture not found: " + split[1] + " in directory: " + parent
												+ " for material: " + f.getName());
								throw new RuntimeException("Material texture not found: " + split[1] + " in directory: "
										+ parent + " for object: " + f.getName());
							}
							currentmat.texture = res.getSakerPath();
							break;
						}
						default: {
							// System.out.println(path + ":" + (linenumber + 1) + ": Warning: Ignored object directive: " + split[0]);
							break;
						}
					}
				}
			}
			synchronized (materials) {
				materials.put(path.toString().replace('\\', '/'), lib);
			}
		}

		MaterialTranslatorTaskFactory.Output result = new MaterialTranslatorTaskFactory.Output(materials);
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, inputCollectionStrategies);
		SerialUtils.writeExternalCollection(out, diffuseToAmbientFactors, SerialUtils::writeExternalCollection);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		inputCollectionStrategies = SerialUtils.readExternalImmutableLinkedHashSet(in);
		diffuseToAmbientFactors = SerialUtils.readExternalImmutableList(in, SerialUtils::readExternalImmutableList);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((inputCollectionStrategies == null) ? 0 : inputCollectionStrategies.hashCode());
		result = prime * result + ((diffuseToAmbientFactors == null) ? 0 : diffuseToAmbientFactors.hashCode());
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
		MaterialTranslatorWorkerTaskFactory other = (MaterialTranslatorWorkerTaskFactory) obj;
		if (inputCollectionStrategies == null) {
			if (other.inputCollectionStrategies != null)
				return false;
		} else if (!inputCollectionStrategies.equals(other.inputCollectionStrategies))
			return false;
		if (diffuseToAmbientFactors == null) {
			if (other.diffuseToAmbientFactors != null)
				return false;
		} else if (!diffuseToAmbientFactors.equals(other.diffuseToAmbientFactors))
			return false;
		return true;
	}

}
