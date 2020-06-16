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
package bence.sipka.compiler.asset;

import java.io.Externalizable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DelegateSakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.utils.StructuredTaskResult;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class AssetsGenerateTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.assets.generate";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("assets");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private SakerPath assetsDirectoryPath;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(SakerPath assetsDirectoryPath) {
			this.assetsDirectoryPath = assetsDirectoryPath;
		}

		public SakerPath getAssetsDirectoryPath() {
			return assetsDirectoryPath;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(assetsDirectoryPath);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			assetsDirectoryPath = SerialUtils.readExternalObject(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assetsDirectoryPath == null) ? 0 : assetsDirectoryPath.hashCode());
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
			if (assetsDirectoryPath == null) {
				if (other.assetsDirectoryPath != null)
					return false;
			} else if (!assetsDirectoryPath.equals(other.assetsDirectoryPath))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "Input" }, required = true)
			public AssetsCompilerTaskFactory.Output inputOption;
			@SakerInput(value = "With")
			public Collection<Object> withOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				//just to retrieve all items
				if (withOption != null) {
					for (Object o : withOption) {
						if (o instanceof StructuredTaskResult) {
							((StructuredTaskResult) o).toResult(taskcontext);
						}
					}
				}

				SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				outputDirectory.clear();
				SakerDirectory assetsdir = outputDirectory.getDirectoryCreate("assets");
				SakerDirectory assetsresdir = assetsdir.getDirectoryCreate("res");

				NavigableMap<String, Integer> assetIdentifiersMap = inputOption.getAssetIdentifiers();

				NavigableMap<String, SakerFile> allmappings = new TreeMap<>();

				for (Entry<String, SakerPath> entry : inputOption.getAssetFiles().entrySet()) {
					SakerFile f = taskcontext.getTaskUtilities().resolveFileAtPath(entry.getValue());
					if (f == null) {
						throw new FileNotFoundException(entry.getValue().toString());
					}

					String assetname = entry.getKey();
					allmappings.put(assetname, f);
					assetsresdir.add(
							new DelegateSakerFile(Integer.toHexString(assetIdentifiersMap.get(entry.getKey())), f));
				}

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(outputDirectory.getFilesRecursiveByPath(
								outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				outputDirectory.synchronize();

				Output result = new Output(assetsdir.getSakerPath());
				taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
				return result;
			}
		};
	}

}
