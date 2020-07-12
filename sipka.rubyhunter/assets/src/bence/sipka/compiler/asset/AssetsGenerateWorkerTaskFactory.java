package bence.sipka.compiler.asset;

import java.io.Externalizable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import bence.sipka.compiler.asset.AssetsGenerateTaskFactory.Output;
import saker.build.file.DelegateSakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class AssetsGenerateWorkerTaskFactory implements TaskFactory<AssetsGenerateTaskFactory.Output>,
		Task<AssetsGenerateTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private AssetsCompilerTaskFactory.Output compilerOutput;

	/**
	 * For {@link Externalizable}.
	 */
	public AssetsGenerateWorkerTaskFactory() {
	}

	public AssetsGenerateWorkerTaskFactory(bence.sipka.compiler.asset.AssetsCompilerTaskFactory.Output inputOption) {
		this.compilerOutput = inputOption;
	}

	@Override
	public Task<? extends Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(AssetsGenerateTaskFactory.TASK_NAME);

		SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(AssetsGenerateTaskFactory.TASK_NAME);
		outputDirectory.clear();
		SakerDirectory assetsdir = outputDirectory.getDirectoryCreate("assets");
		SakerDirectory assetsresdir = assetsdir.getDirectoryCreate("res");

		NavigableMap<String, Integer> assetIdentifiersMap = compilerOutput.getAssetIdentifiers();

		NavigableMap<String, SakerFile> allmappings = new TreeMap<>();

		for (Entry<String, SakerPath> entry : compilerOutput.getAssetFiles().entrySet()) {
			SakerFile f = taskcontext.getTaskUtilities().resolveFileAtPath(entry.getValue());
			if (f == null) {
				throw new FileNotFoundException(entry.getValue().toString());
			}
			
			taskcontext.reportInputFileDependency(null, entry.getValue(), f.getContentDescriptor());

			String assetname = entry.getKey();
			allmappings.put(assetname, f);
			assetsresdir.add(new DelegateSakerFile(Integer.toHexString(assetIdentifiersMap.get(entry.getKey())), f));
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(outputDirectory
				.getFilesRecursiveByPath(outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		outputDirectory.synchronize();

		Output result = new Output(assetsdir.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(compilerOutput);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		compilerOutput = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((compilerOutput == null) ? 0 : compilerOutput.hashCode());
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
		AssetsGenerateWorkerTaskFactory other = (AssetsGenerateWorkerTaskFactory) obj;
		if (compilerOutput == null) {
			if (other.compilerOutput != null)
				return false;
		} else if (!compilerOutput.equals(other.compilerOutput))
			return false;
		return true;
	}

}
