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
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.TypeDeclaration;
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
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class AssetsCompilerTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.assets.compile";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("assets");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, Integer> assetIdentifiers;

		private NavigableMap<String, TypeDeclaration> typeDeclarations;

		private NavigableMap<String, SakerPath> assetFiles;

		private SakerPath sourceDirectory;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, Integer> assetIdentifiers,
				NavigableMap<String, TypeDeclaration> typeDeclarations, NavigableMap<String, SakerPath> assetFiles,
				SakerPath sourceDirectory) {
			this.sourceDirectory = sourceDirectory;
			this.assetIdentifiers = ImmutableUtils.makeImmutableNavigableMap(assetIdentifiers);
			this.typeDeclarations = ImmutableUtils.makeImmutableNavigableMap(typeDeclarations);
			this.assetFiles = ImmutableUtils.makeImmutableNavigableMap(assetFiles);
		}

		public NavigableMap<String, Integer> getAssetIdentifiers() {
			return assetIdentifiers;
		}

		public NavigableMap<String, TypeDeclaration> getTypeDeclarations() {
			return typeDeclarations;
		}

		public NavigableMap<String, SakerPath> getAssetFiles() {
			return assetFiles;
		}

		public SakerPath getSourceDirectory() {
			return sourceDirectory;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(sourceDirectory);
			SerialUtils.writeExternalMap(out, typeDeclarations);
			SerialUtils.writeExternalMap(out, assetIdentifiers);
			SerialUtils.writeExternalMap(out, assetFiles);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			sourceDirectory = SerialUtils.readExternalObject(in);
			typeDeclarations = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			assetIdentifiers = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			assetFiles = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "Input" }, required = true)
			public AssetsAllocatorTaskFactory.Output inputOption;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				outputDirectory.clear();
				SakerDirectory sourcesdir = outputDirectory.getDirectoryCreate("sources");
				SakerDirectory gendir = sourcesdir.getDirectoryCreate("gen");

				NavigableMap<String, Integer> assetIdentifiersMap = new TreeMap<>();

				NavigableMap<String, SakerPath> allmappings = new TreeMap<>();
				for (Entry<String, Entry<SakerPath, Integer>> entry : inputOption.getAssetIdentifiers().entrySet()) {
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

				gendir.add(new TemplatedSourceModularFile("assets.h",
						new TemplatedSource(descriptor::getInputStream, "gen/assets.h")).setThis(assetIdentifiersMap));

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(outputDirectory.getFilesRecursiveByPath(
								outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				outputDirectory.synchronize();

				NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
				AssetTypeDeclaration assettype = new AssetTypeDeclaration(assetIdentifiersMap);
				typeDeclarations.put(assettype.getName(), assettype);

				return new Output(assetIdentifiersMap, typeDeclarations, allmappings, sourcesdir.getSakerPath());
			}
		};
	}

}
