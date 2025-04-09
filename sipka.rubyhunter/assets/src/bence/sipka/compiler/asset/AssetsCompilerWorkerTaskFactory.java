package bence.sipka.compiler.asset;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class AssetsCompilerWorkerTaskFactory implements TaskFactory<AssetsCompilerTaskFactory.Output>,
		Task<AssetsCompilerTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("assets");

	private AssetsAllocatorTaskFactory.Output allocatorOutput;

	/**
	 * For {@link Externalizable}.
	 */
	public AssetsCompilerWorkerTaskFactory() {
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public AssetsCompilerTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(AssetsCompilerTaskFactory.TASK_NAME);

		SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(AssetsCompilerTaskFactory.TASK_NAME);
		outputDirectory.clear();
		SakerDirectory sourcesdir = outputDirectory.getDirectoryCreate("sources");
		SakerDirectory gendir = sourcesdir.getDirectoryCreate("gen");

		NavigableMap<String, Integer> assetIdentifiersMap = new TreeMap<>();

		NavigableMap<String, SakerPath> allmappings = new TreeMap<>();
		for (Entry<String, Entry<SakerPath, Integer>> entry : allocatorOutput.getAssetIdentifiers().entrySet()) {
			int assetid = entry.getValue().getValue();
			String assetname = entry.getKey();
			assetIdentifiersMap.put(assetname, assetid);
			allmappings.put(assetname, entry.getValue().getKey());
		}

		int idx = 0;
		for (Entry<String, ?> entry : allmappings.entrySet()) {
			String assetname = entry.getKey();
			int assetid = idx++;
			assetIdentifiersMap.put(assetname, assetid);
		}

		gendir.add(new TemplatedSourceSakerFile("assets.h",
				new TemplatedSource(descriptor::getInputStream, "gen/assets.h")).setThis(assetIdentifiersMap));

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(outputDirectory
				.getFilesRecursiveByPath(outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		outputDirectory.synchronize();

		NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
		AssetTypeDeclaration assettype = new AssetTypeDeclaration(assetIdentifiersMap);
		typeDeclarations.put(assettype.getName(), assettype);

		return new AssetsCompilerTaskFactory.Output(assetIdentifiersMap, typeDeclarations, allmappings,
				sourcesdir.getSakerPath());
	}

	public AssetsCompilerWorkerTaskFactory(bence.sipka.compiler.asset.AssetsAllocatorTaskFactory.Output inputOption) {
		this.allocatorOutput = inputOption;
	}

	@Override
	public Task<? extends AssetsCompilerTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(allocatorOutput);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		allocatorOutput = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((allocatorOutput == null) ? 0 : allocatorOutput.hashCode());
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
		AssetsCompilerWorkerTaskFactory other = (AssetsCompilerWorkerTaskFactory) obj;
		if (allocatorOutput == null) {
			if (other.allocatorOutput != null)
				return false;
		} else if (!allocatorOutput.equals(other.allocatorOutput))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "AssetsCompilerWorkerTaskFactory[" + (allocatorOutput != null ? "inputOption=" + allocatorOutput : "")
				+ "]";
	}
}
