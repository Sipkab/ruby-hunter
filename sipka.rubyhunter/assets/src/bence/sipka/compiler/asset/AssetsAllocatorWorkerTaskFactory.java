package bence.sipka.compiler.asset;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import bence.sipka.compiler.asset.AssetsAllocatorTaskFactory.Output;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class AssetsAllocatorWorkerTaskFactory implements TaskFactory<AssetsAllocatorTaskFactory.Output>,
		Task<AssetsAllocatorTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("assets");

	private List<Map<String, SakerPath>> externalAssetsOption;

	private Set<FileCollectionStrategy> inputOption;

	/**
	 * For {@link Externalizable}.
	 */
	public AssetsAllocatorWorkerTaskFactory() {
	}

	public AssetsAllocatorWorkerTaskFactory(List<Map<String, SakerPath>> externalAssetsOption,
			Set<FileCollectionStrategy> inputOption) {
		this.externalAssetsOption = externalAssetsOption;
		this.inputOption = inputOption;
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
		taskcontext.setStandardOutDisplayIdentifier(AssetsAllocatorTaskFactory.TASK_NAME);

		NavigableMap<String, Integer> assetIdentifiersMap = new TreeMap<>();

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputOption);

		NavigableMap<String, SakerPath> allmappings = new TreeMap<>();

		SakerPath workingdirpath = taskcontext.getTaskWorkingDirectoryPath();
		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			if (entry.getValue() instanceof SakerDirectory) {
				continue;
			}
			String assetid = workingdirpath.relativize(entry.getKey()).toString();
			allmappings.put(assetid, entry.getKey());
		}

		for (Map<String, SakerPath> assetmap : externalAssetsOption) {
			for (Entry<String, SakerPath> entry : assetmap.entrySet()) {
				SakerPath assetpath = entry.getValue();
				SakerPathFiles.requireAbsolutePath(assetpath);

				SakerPath prev = allmappings.put(entry.getKey(), assetpath);
				if (prev != null && !prev.equals(assetpath)) {
					throw new IllegalArgumentException(
							"Asset name mismatch on: " + entry.getKey() + " with " + prev + " and " + assetpath);
				}
			}
		}

		NavigableMap<String, Entry<SakerPath, Integer>> assetentrymap = new TreeMap<>();

		NavigableMap<Integer, SakerPath> identifierpaths = new TreeMap<>();
		NavigableMap<SakerPath, Integer> pathidentifiers = new TreeMap<>();
		int idx = 0;
		for (Entry<String, SakerPath> entry : allmappings.entrySet()) {
			String assetname = entry.getKey();
			SakerPath assetpath = entry.getValue();

			int assetid;
			Integer presentidx = pathidentifiers.get(assetpath);
			if (presentidx != null) {
				assetid = presentidx;
			} else {
				assetid = idx++;
				pathidentifiers.put(assetpath, assetid);
			}

			assetIdentifiersMap.put(assetname, assetid);
			identifierpaths.put(assetid, assetpath);
			assetentrymap.put(assetname, ImmutableUtils.makeImmutableMapEntry(assetpath, assetid));
		}
		return new Output(assetentrymap);
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, externalAssetsOption, SerialUtils::writeExternalMap);
		SerialUtils.writeExternalCollection(out, inputOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		externalAssetsOption = SerialUtils.readExternalCollection(new ArrayList<>(), in,
				SerialUtils::readExternalImmutableLinkedHashMap);
		inputOption = SerialUtils.readExternalImmutableLinkedHashSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((externalAssetsOption == null) ? 0 : externalAssetsOption.hashCode());
		result = prime * result + ((inputOption == null) ? 0 : inputOption.hashCode());
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
		AssetsAllocatorWorkerTaskFactory other = (AssetsAllocatorWorkerTaskFactory) obj;
		if (externalAssetsOption == null) {
			if (other.externalAssetsOption != null)
				return false;
		} else if (!externalAssetsOption.equals(other.externalAssetsOption))
			return false;
		if (inputOption == null) {
			if (other.inputOption != null)
				return false;
		} else if (!inputOption.equals(other.inputOption))
			return false;
		return true;
	}
}
