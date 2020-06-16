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
package bence.sipka.compiler.logmodule;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.TypesTaskFactory;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.annot.DataContext;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class LogSourcesTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.log.sources";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("log");

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

			@DataContext
			public LogSourceData logData = new LogSourceData();

			@SakerInput("PlatformName")
			public String PlatformName;

			@SakerInput(value = "Types", required = true)
			public TypesTaskFactory.Output types;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				if (logData.enabled == null) {
					logData.enabled = logData.Debug;
				}
				logData.typeDeclarations = types.getTypeDeclarations();

				SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				outputDirectory.clear();

				SakerDirectory gendir = outputDirectory.getDirectoryCreate("gen");
				gendir.add(new TemplatedSourceModularFile("log.h", new TemplatedSource(descriptor::getInputStream,
						logData.enabled ? "gen/log.h" : "gen/log_release.h")).setThis(logData));
				gendir.add(new TemplatedSourceModularFile("log.cpp",
						new TemplatedSource(descriptor::getInputStream, "gen/log.cpp")).setThis(logData));

				if ("ios".equalsIgnoreCase(PlatformName) || "macosx".equalsIgnoreCase(PlatformName)) {
					TemplatedSourceModularFile logmm = new TemplatedSourceModularFile("log.mm",
							new TemplatedSource(descriptor::getInputStream, "gen/log.mm")).setThis(logData);
					gendir.add(logmm);
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(outputDirectory.getFilesRecursiveByPath(
								outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				outputDirectory.synchronize();

				Output result = new Output(outputDirectory.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}

	public static class LogSourceData implements Externalizable {
		private static final long serialVersionUID = 1L;

		@SakerInput(value = "Debug")
		public boolean Debug = false;

		@SakerInput(value = "DebugNew")
		public boolean debugNew = false;

		@SakerInput(value = "SupportDebugNew")
		public boolean supportDebugNew = false;

		@SakerInput(value = "Enabled")
		public Boolean enabled = null;

		@SakerInput(value = "LogDebugNew")
		public boolean logDebugNew = true;

		public Map<String, TypeDeclaration> typeDeclarations;

		/**
		 * For {@link Externalizable}.
		 */
		public LogSourceData() {
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeBoolean(Debug);
			out.writeBoolean(debugNew);
			out.writeBoolean(supportDebugNew);
			out.writeBoolean(enabled);
			out.writeBoolean(logDebugNew);
			SerialUtils.writeExternalMap(out, typeDeclarations);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			Debug = in.readBoolean();
			debugNew = in.readBoolean();
			supportDebugNew = in.readBoolean();
			enabled = in.readBoolean();
			logDebugNew = in.readBoolean();
			typeDeclarations = SerialUtils.readExternalImmutableNavigableMap(in);
		}

		@Override
		public String toString() {
			return "LogSourceData [Debug=" + Debug + ", debugNew=" + debugNew + ", supportDebugNew=" + supportDebugNew
					+ ", " + (enabled != null ? "enabled=" + enabled + ", " : "") + "logDebugNew=" + logDebugNew + ", "
					+ (typeDeclarations != null ? "typeDeclarations=" + typeDeclarations : "") + "]";
		}

	}
}
