package bence.sipka.compiler.logmodule;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.LinkedHashMap;
import java.util.Objects;

import bence.sipka.compiler.logmodule.LogSourcesTaskFactory.Output;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class LogSourcesWorkerTaskFactory
		implements TaskFactory<LogSourcesTaskFactory.Output>, Task<LogSourcesTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("log");

	private LogSourceData logData;

	/**
	 * For {@link Externalizable}.
	 */
	public LogSourcesWorkerTaskFactory() {
	}

	public LogSourcesWorkerTaskFactory(LogSourceData logData) {
		Objects.requireNonNull(logData, "logData");
		this.logData = logData;
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
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Platform", logData.PlatformName);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(LogSourcesTaskFactory.TASK_NAME);

		SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(LogSourcesTaskFactory.TASK_NAME);
		outputDirectory.clear();

		SakerDirectory gendir = outputDirectory.getDirectoryCreate("gen");
		gendir.add(new TemplatedSourceSakerFile("log.h",
				new TemplatedSource(descriptor::getInputStream, logData.enabled ? "gen/log.h" : "gen/log_release.h"))
						.setThis(logData));
		gendir.add(
				new TemplatedSourceSakerFile("log.cpp", new TemplatedSource(descriptor::getInputStream, "gen/log.cpp"))
						.setThis(logData));

		if ("ios".equalsIgnoreCase(logData.PlatformName) || "macosx".equalsIgnoreCase(logData.PlatformName)) {
			TemplatedSourceSakerFile logmm = new TemplatedSourceSakerFile("log.mm",
					new TemplatedSource(descriptor::getInputStream, "gen/log.mm")).setThis(logData);
			gendir.add(logmm);
		}

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(outputDirectory
				.getFilesRecursiveByPath(outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		outputDirectory.synchronize();

		Output result = new Output(outputDirectory.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(logData);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		logData = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((logData == null) ? 0 : logData.hashCode());
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
		LogSourcesWorkerTaskFactory other = (LogSourcesWorkerTaskFactory) obj;
		if (logData == null) {
			if (other.logData != null)
				return false;
		} else if (!logData.equals(other.logData))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "LogSourcesWorkerTaskFactory[" + (logData != null ? "logData=" + logData : "") + "]";
	}

}
