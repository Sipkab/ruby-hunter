package bence.sipka.rhfw;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.rhfw.RhProjectTaskFactory.Output;
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
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class RhProjectWorkerTaskFactory
		implements TaskFactory<RhProjectTaskFactory.Output>, Task<RhProjectTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("project");

	private String id = "";
	private boolean debugOption;
	private String applicationNameOption;

	/**
	 * For {@link Externalizable}.
	 */
	public RhProjectWorkerTaskFactory() {
	}

	public RhProjectWorkerTaskFactory(String id, boolean debugOption, String applicationNameOption) {
		this.id = id;
		this.debugOption = debugOption;
		this.applicationNameOption = applicationNameOption;
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
		taskcontext.setStandardOutDisplayIdentifier(RhProjectTaskFactory.TASK_NAME);

		ConfigHeaderData headerdata = new ConfigHeaderData(debugOption, applicationNameOption);

		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(RhProjectTaskFactory.TASK_NAME);
		SakerDirectory includedirectory = buildDirectory.getDirectoryCreate(
				(ObjectUtils.isNullOrEmpty(id) ? "" : id + "-") + "sources" + (debugOption ? "-debug" : ""));

		includedirectory.clear();
		SakerDirectory sourcesgendirectory = includedirectory.getDirectoryCreate("gen");

		sourcesgendirectory.add(new TemplatedSourceSakerFile("configuration.h",
				new TemplatedSource(descriptor::getInputStream, "gen/configuration.h")).setThis(headerdata));

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(includedirectory
				.getFilesRecursiveByPath(includedirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		includedirectory.synchronize();

		Output result = new Output(includedirectory.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	public static class ConfigHeaderData implements Externalizable {
		private static final long serialVersionUID = 1L;

		public boolean debug = false;
		public String defaultApplicationName;

		/**
		 * For {@link Externalizable}.
		 */
		public ConfigHeaderData() {
		}

		public ConfigHeaderData(boolean debug, String defaultApplicationName) {
			this.debug = debug;
			this.defaultApplicationName = defaultApplicationName;
		}

		public String getDebugValue() {
			return debug ? "1" : "0";
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeBoolean(debug);
			out.writeObject(defaultApplicationName);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			debug = in.readBoolean();
			defaultApplicationName = SerialUtils.readExternalObject(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + (debug ? 1231 : 1237);
			result = prime * result + ((defaultApplicationName == null) ? 0 : defaultApplicationName.hashCode());
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
			ConfigHeaderData other = (ConfigHeaderData) obj;
			if (debug != other.debug)
				return false;
			if (defaultApplicationName == null) {
				if (other.defaultApplicationName != null)
					return false;
			} else if (!defaultApplicationName.equals(other.defaultApplicationName))
				return false;
			return true;
		}

		@Override
		public String toString() {
			return "ConfigHeaderData[debug=" + debug + ", "
					+ (defaultApplicationName != null ? "defaultApplicationName=" + defaultApplicationName : "") + "]";
		}
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(id);
		out.writeBoolean(debugOption);
		out.writeObject(applicationNameOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		id = SerialUtils.readExternalObject(in);
		debugOption = in.readBoolean();
		applicationNameOption = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((applicationNameOption == null) ? 0 : applicationNameOption.hashCode());
		result = prime * result + (debugOption ? 1231 : 1237);
		result = prime * result + ((id == null) ? 0 : id.hashCode());
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
		RhProjectWorkerTaskFactory other = (RhProjectWorkerTaskFactory) obj;
		if (applicationNameOption == null) {
			if (other.applicationNameOption != null)
				return false;
		} else if (!applicationNameOption.equals(other.applicationNameOption))
			return false;
		if (debugOption != other.debugOption)
			return false;
		if (id == null) {
			if (other.id != null)
				return false;
		} else if (!id.equals(other.id))
			return false;
		return true;
	}

}
