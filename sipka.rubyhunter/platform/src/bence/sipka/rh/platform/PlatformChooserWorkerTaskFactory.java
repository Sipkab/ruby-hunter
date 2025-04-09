package bence.sipka.rh.platform;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.LinkedHashMap;

import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.identifier.TaskIdentifier;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class PlatformChooserWorkerTaskFactory implements TaskFactory<PlatformChooserTaskFactory.Output>,
		Task<PlatformChooserTaskFactory.Output>, Externalizable, TaskIdentifier {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("platform");

	private String platformName;

	/**
	 * For {@link Externalizable}.
	 */
	public PlatformChooserWorkerTaskFactory() {
	}

	public PlatformChooserWorkerTaskFactory(String platformName) {
		this.platformName = platformName;
	}

	@Override
	public Task<? extends PlatformChooserTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public PlatformChooserTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Platform", platformName);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(PlatformChooserTaskFactory.TASK_NAME);
		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(PlatformChooserTaskFactory.TASK_NAME);

		SakerDirectory sourcedir;

		switch (platformName) {
			case "win32": {
				sourcedir = buildDirectory.getDirectoryCreate("win32");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(new ByteArraySakerFile("platform.h", descriptor.getBytes("win32/sources/gen/platform.h")));
				break;
			}
			case "android": {
				sourcedir = buildDirectory.getDirectoryCreate("android");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(new ByteArraySakerFile("platform.h", descriptor.getBytes("android/sources/gen/platform.h")));
				break;
			}
			case "winstore": {
				sourcedir = buildDirectory.getDirectoryCreate("winstore");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(
						new ByteArraySakerFile("platform.h", descriptor.getBytes("winstore/sources/gen/platform.h")));
				break;
			}
			case "ios": {
				sourcedir = buildDirectory.getDirectoryCreate("ios");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(new ByteArraySakerFile("platform.h", descriptor.getBytes("ios/sources/gen/platform.h")));
				break;
			}
			case "macosx": {
				sourcedir = buildDirectory.getDirectoryCreate("macosx");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(new ByteArraySakerFile("platform.h", descriptor.getBytes("macosx/sources/gen/platform.h")));
				break;
			}
			case "linux": {
				sourcedir = buildDirectory.getDirectoryCreate("linux");
				sourcedir.clear();
				SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
				gendir.add(new ByteArraySakerFile("platform.h", descriptor.getBytes("linux/sources/gen/platform.h")));
				gendir.add(new ByteArraySakerFile("OpenALCommonHeader.h",
						descriptor.getBytes("linux/sources/gen/OpenALCommonHeader.h")));
				break;
			}
			default: {
				throw new IllegalArgumentException(platformName);
			}
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(
				sourcedir.getFilesRecursiveByPath(sourcedir.getSakerPath(), DirectoryVisitPredicate.everything())));
		sourcedir.synchronize();

		PlatformChooserTaskFactory.Output result = new PlatformChooserTaskFactory.Output(sourcedir.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(platformName);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		platformName = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((platformName == null) ? 0 : platformName.hashCode());
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
		PlatformChooserWorkerTaskFactory other = (PlatformChooserWorkerTaskFactory) obj;
		if (platformName == null) {
			if (other.platformName != null)
				return false;
		} else if (!platformName.equals(other.platformName))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "PlatformChooserWorkerTaskFactory[" + (platformName != null ? "platformName=" + platformName : "") + "]";
	}

}
