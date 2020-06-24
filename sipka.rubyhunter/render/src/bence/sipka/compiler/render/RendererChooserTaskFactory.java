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
package bence.sipka.compiler.render;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Collections;
import java.util.Locale;
import java.util.Map;
import java.util.NavigableMap;

import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.utils.RHFrontendParameterizableTask;
import saker.build.file.path.SakerPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.identifier.TaskIdentifier;
import saker.build.task.utils.TaskBuilderResult;
import saker.build.task.utils.annot.SakerInput;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class RendererChooserTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.renderer.choose";

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, TypeDeclaration> typeDeclarations;
		private SakerPath sourceDirectory;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, TypeDeclaration> typeDeclarations, SakerPath genDirectory) {
			this.sourceDirectory = genDirectory;
			this.typeDeclarations = ImmutableUtils.makeImmutableNavigableMap(typeDeclarations);
		}

		public Map<String, TypeDeclaration> getTypeDeclarations() {
			return typeDeclarations;
		}

		public SakerPath getSourceDirectory() {
			return sourceDirectory;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(sourceDirectory);
			SerialUtils.writeExternalMap(out, typeDeclarations);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			sourceDirectory = SerialUtils.readExternalObject(in);
			typeDeclarations = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((sourceDirectory == null) ? 0 : sourceDirectory.hashCode());
			result = prime * result + ((typeDeclarations == null) ? 0 : typeDeclarations.hashCode());
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
			if (typeDeclarations == null) {
				if (other.typeDeclarations != null)
					return false;
			} else if (!typeDeclarations.equals(other.typeDeclarations))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new RHFrontendParameterizableTask() {
			@SakerInput(value = { "", "RenderAPI" }, required = true)
			public Collection<String> renderapi = Collections.emptyList();

			@SakerInput(value = { "PlatformName" })
			public String platformNameOption;

			@Override
			protected TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext) {
				RendererChooserWorkerTaskFactory workertask = new RendererChooserWorkerTaskFactory(
						platformNameOption.toLowerCase(Locale.ENGLISH),
						ImmutableUtils.makeImmutableNavigableSet(renderapi));
				return TaskBuilderResult.create(
						TaskIdentifier.builder(RendererChooserWorkerTaskFactory.class.getName()).build(), workertask);
			}
		};
	}
}
