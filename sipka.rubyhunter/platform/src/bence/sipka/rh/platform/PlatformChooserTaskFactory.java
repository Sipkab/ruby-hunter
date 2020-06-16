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
package bence.sipka.rh.platform;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Locale;

import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class PlatformChooserTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.platform.choose";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("platform");

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
			@SakerInput(value = { "", "PlatformName" }, required = true)
			public String platformNameOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);

				SakerDirectory sourcedir;

				switch (platformNameOption.toLowerCase(Locale.ENGLISH)) {
					case "win32": {
						sourcedir = buildDirectory.getDirectoryCreate("win32");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("win32/sources/gen/platform.h")));
						break;
					}
					case "android": {
						sourcedir = buildDirectory.getDirectoryCreate("android");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("android/sources/gen/platform.h")));
						break;
					}
					case "winstore": {
						sourcedir = buildDirectory.getDirectoryCreate("winstore");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("winstore/sources/gen/platform.h")));
						break;
					}
					case "ios": {
						sourcedir = buildDirectory.getDirectoryCreate("ios");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("ios/sources/gen/platform.h")));
						break;
					}
					case "macosx": {
						sourcedir = buildDirectory.getDirectoryCreate("macosx");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("macosx/sources/gen/platform.h")));
						break;
					}
					case "linux": {
						sourcedir = buildDirectory.getDirectoryCreate("linux");
						sourcedir.clear();
						SakerDirectory gendir = sourcedir.getDirectoryCreate("gen");
						gendir.add(new ByteArraySakerFile("platform.h",
								descriptor.getBytes("linux/sources/gen/platform.h")));
						gendir.add(new ByteArraySakerFile("OpenALCommonHeader.h",
								descriptor.getBytes("linux/sources/gen/OpenALCommonHeader.h")));
						break;
					}
					default: {
						throw new UnsupportedOperationException(platformNameOption);
					}
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(sourcedir.getFilesRecursiveByPath(sourcedir.getSakerPath(),
								DirectoryVisitPredicate.everything())));
				sourcedir.synchronize();

				Output result = new Output(sourcedir.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}

}
