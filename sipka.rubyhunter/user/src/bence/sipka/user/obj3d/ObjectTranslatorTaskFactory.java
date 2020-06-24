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
package bence.sipka.user.obj3d;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
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
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class ObjectTranslatorTaskFactory extends FrontendTaskFactory<Object> {

	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.obj3d.translate";

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private ObjectCollection objectCollection;
		private NavigableMap<String, SakerPath> assets;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(ObjectCollection objectCollection, NavigableMap<String, SakerPath> assets) {
			this.objectCollection = objectCollection;
			this.assets = ImmutableUtils.makeImmutableNavigableMap(assets);
		}

		public ObjectCollection getObjectCollection() {
			return objectCollection;
		}

		public NavigableMap<String, SakerPath> getAssets() {
			return assets;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(objectCollection);
			SerialUtils.writeExternalMap(out, assets);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			objectCollection = SerialUtils.readExternalObject(in);
			assets = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assets == null) ? 0 : assets.hashCode());
			result = prime * result + ((objectCollection == null) ? 0 : objectCollection.hashCode());
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
			if (assets == null) {
				if (other.assets != null)
					return false;
			} else if (!assets.equals(other.assets))
				return false;
			if (objectCollection == null) {
				if (other.objectCollection != null)
					return false;
			} else if (!objectCollection.equals(other.objectCollection))
				return false;
			return true;
		}

	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new RHFrontendParameterizableTask() {
			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@SakerInput(value = "Materials", required = true)
			public MaterialTranslatorTaskFactory.Output materialsOption;

			@Override
			protected TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext) {
				Set<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				return TaskBuilderResult.create(
						TaskIdentifier.builder(ObjectTranslatorWorkerTaskFactory.class.getName()).build(),
						new ObjectTranslatorWorkerTaskFactory(collectionstrategies, materialsOption));
			}
		};
	}

}
