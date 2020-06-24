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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;

import bence.sipka.utils.RHFrontendParameterizableTask;
import saker.build.file.path.SakerPath;
import saker.build.file.path.WildcardPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.identifier.TaskIdentifier;
import saker.build.task.utils.TaskBuilderResult;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.ObjectUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class AssetsAllocatorTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.assets.allocate";

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

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assetIdentifiers == null) ? 0 : assetIdentifiers.hashCode());
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
			if (assetIdentifiers == null) {
				if (other.assetIdentifiers != null)
					return false;
			} else if (!assetIdentifiers.equals(other.assetIdentifiers))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new RHFrontendParameterizableTask() {
			@SakerInput(value = "ExternalAssets")
			public List<Map<String, SakerPath>> externalAssetsOption = Collections.emptyList();

			@SakerInput(value = { "", "Input" })
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@Override
			protected TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext) {
				Set<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				ArrayList<Map<String, SakerPath>> externalassets = ObjectUtils.cloneArrayList(externalAssetsOption,
						ObjectUtils::cloneTreeMap);

				return TaskBuilderResult.create(
						TaskIdentifier.builder(AssetsAllocatorWorkerTaskFactory.class.getName()).build(),
						new AssetsAllocatorWorkerTaskFactory(externalassets, collectionstrategies));
			}
		};
	}

}
