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
package bence.sipka.rhfw;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class RhProjectTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.project.configure";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("project");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private SakerPath sourceDirectory;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(SakerPath sourceDirectory) {
			this.sourceDirectory = sourceDirectory;
		}

		public SakerPath getSourceDirectory() {
			return sourceDirectory;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(sourceDirectory);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			sourceDirectory = SerialUtils.readExternalObject(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((sourceDirectory == null) ? 0 : sourceDirectory.hashCode());
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
			Output other = (Output) obj;
			if (sourceDirectory == null) {
				if (other.sourceDirectory != null)
					return false;
			} else if (!sourceDirectory.equals(other.sourceDirectory))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = "Identifier")
			public String id = "";

			@SakerInput(value = "Debug")
			public boolean debugOption;

			@SakerInput(value = "ApplicationName")
			public ApplicationNames applicationNameOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				ConfigHeaderData headerdata = new ConfigHeaderData(debugOption, applicationNameOption.getDefault());

				SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				SakerDirectory includedirectory = buildDirectory.getDirectoryCreate(
						(ObjectUtils.isNullOrEmpty(id) ? "" : id + "-") + "sources" + (debugOption ? "-debug" : ""));

				includedirectory.clear();
				SakerDirectory sourcesgendirectory = includedirectory.getDirectoryCreate("gen");

				sourcesgendirectory.add(new TemplatedSourceModularFile("configuration.h",
						new TemplatedSource(descriptor::getInputStream, "gen/configuration.h")).setThis(headerdata));

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(includedirectory.getFilesRecursiveByPath(
								includedirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				includedirectory.synchronize();

				Output result = new Output(includedirectory.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
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
}
