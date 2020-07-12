package bence.sipka.user.obj3d;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import bence.sipka.compiler.asset.AssetsAllocatorTaskFactory;
import bence.sipka.user.obj3d.ObjectCollection.DuplicateObjectData;
import bence.sipka.user.obj3d.ObjectTranslatorTaskFactory.Output;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class ObjectLinkerWorkerTaskFactory implements TaskFactory<Object>, Task<Object>, Externalizable {
	private static final long serialVersionUID = 1L;

	private ObjectTranslatorTaskFactory.Output translaterOutputOption;

	private AssetsAllocatorTaskFactory.Output assetsOption;

	/**
	 * For {@link Externalizable}.
	 */
	public ObjectLinkerWorkerTaskFactory() {
	}

	public ObjectLinkerWorkerTaskFactory(Output translaterOutputOption,
			bence.sipka.compiler.asset.AssetsAllocatorTaskFactory.Output assetsOption) {
		this.translaterOutputOption = translaterOutputOption;
		this.assetsOption = assetsOption;
	}

	@Override
	public Task<? extends Object> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public Object run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(ObjectLinkerTaskFactory.TASK_NAME);

		SakerDirectory genDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(ObjectLinkerTaskFactory.TASK_NAME);
		genDirectory.clear();

		Map<String, Entry<SakerPath, Integer>> assetsIdentifierMap = assetsOption.getAssetIdentifiers();
		Map<String, Integer> simpleAssetsIdentifierMap = new TreeMap<>();

		for (Entry<String, Entry<SakerPath, Integer>> entry : assetsIdentifierMap.entrySet()) {
			simpleAssetsIdentifierMap.put(entry.getKey(), entry.getValue().getValue());
		}

		ObjectCollection objcoll = translaterOutputOption.getObjectCollection();
		ObjectConfiguration objconfig = new ObjectConfiguration(objcoll);
		ByteArrayRegion data = objconfig.getBytes(taskcontext.getTaskWorkingDirectoryPath(), simpleAssetsIdentifierMap);

		for (ObjectData objdata : objcoll.objects) {
			String meshfilename = objdata.fileName + ".mesh";
			genDirectory.add(new ObjectSakerFile(meshfilename, objconfig, objdata));
		}

		for (DuplicateObjectData dd : objcoll.duplicateDatas) {
			genDirectory.add(new DuplicatObjectModularFile(dd.fileName, objconfig, dd.objectDataId, dd.objectDataOrigin,
					dd.materialLibrary));
		}

		SakerFile objcollfile = new ByteArraySakerFile("objects_3d_collection", data);
		genDirectory.add(objcollfile);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(genDirectory
				.getFilesRecursiveByPath(genDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		genDirectory.synchronize();

		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(translaterOutputOption);
		out.writeObject(assetsOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		translaterOutputOption = SerialUtils.readExternalObject(in);
		assetsOption = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((assetsOption == null) ? 0 : assetsOption.hashCode());
		result = prime * result + ((translaterOutputOption == null) ? 0 : translaterOutputOption.hashCode());
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
		ObjectLinkerWorkerTaskFactory other = (ObjectLinkerWorkerTaskFactory) obj;
		if (assetsOption == null) {
			if (other.assetsOption != null)
				return false;
		} else if (!assetsOption.equals(other.assetsOption))
			return false;
		if (translaterOutputOption == null) {
			if (other.translaterOutputOption != null)
				return false;
		} else if (!translaterOutputOption.equals(other.translaterOutputOption))
			return false;
		return true;
	}

}
