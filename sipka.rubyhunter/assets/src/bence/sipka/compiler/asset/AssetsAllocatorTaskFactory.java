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
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.path.WildcardPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class AssetsAllocatorTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.assets.allocate";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("assets");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, Entry<SakerPath, Integer>> assetIdentifiers;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, Entry<SakerPath, Integer>> assetIdentifiers) {
			this.assetIdentifiers = ImmutableUtils.makeImmutableNavigableMap(assetIdentifiers);
		}

		public NavigableMap<String, Entry<SakerPath, Integer>> getAssetIdentifiers() {
			return assetIdentifiers;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			SerialUtils.writeExternalMap(out, assetIdentifiers);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			assetIdentifiers = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = "ExternalAssets")
			public List<Map<String, SakerPath>> externalAssetsOption = Collections.emptyList();

			@SakerInput(value = { "", "Input" })
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				NavigableMap<String, Integer> assetIdentifiersMap = new TreeMap<>();

				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				externalAssetsOption = ObjectUtils.cloneArrayList(externalAssetsOption, ObjectUtils::cloneTreeMap);

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

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
							throw new IllegalArgumentException("Asset name mismatch on: " + entry.getKey() + " with "
									+ prev + " and " + assetpath);
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
		};
	}

}
