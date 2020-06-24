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
package bence.sipka.compiler.xml;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import bence.sipka.compiler.xml.declarations.ElementDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
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
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class XmlParseTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	public static final String TASK_NAME = "sipka.rh.xml.parse";

	public static final BundleResourceSupplier descriptor = BundleContentAccess
			.getBundleResourceSupplier("xml_compiler");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		NavigableSet<SakerPath> xmls = new TreeSet<>();
		NavigableMap<String, SakerPath> assets = new TreeMap<>();

		NavigableMap<String, Integer> elementIdentifiers = new TreeMap<>();
		NavigableMap<String, Integer> attributeIdentifiers = new TreeMap<>();
		NavigableMap<String, Integer> userIdentifiers = new TreeMap<>();
		NavigableMap<String, Boolean> xmlElementAbstractionMap = new TreeMap<>();
		NavigableMap<String, ElementDeclaration> xmlelements = new TreeMap<>();

		public Output() {
		}

		public NavigableMap<String, SakerPath> getAssets() {
			return assets;
		}

		public NavigableMap<String, Integer> getUserIdentifiers() {
			return userIdentifiers;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			SerialUtils.writeExternalCollection(out, xmls);
			SerialUtils.writeExternalMap(out, assets);

			SerialUtils.writeExternalMap(out, elementIdentifiers);
			SerialUtils.writeExternalMap(out, attributeIdentifiers);
			SerialUtils.writeExternalMap(out, userIdentifiers);
			SerialUtils.writeExternalMap(out, xmlElementAbstractionMap);
			SerialUtils.writeExternalMap(out, xmlelements);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			xmls = SerialUtils.readExternalSortedImmutableNavigableSet(in);
			assets = SerialUtils.readExternalSortedImmutableNavigableMap(in);

			elementIdentifiers = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			attributeIdentifiers = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			userIdentifiers = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			xmlElementAbstractionMap = SerialUtils.readExternalSortedImmutableNavigableMap(in);
			xmlelements = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((assets == null) ? 0 : assets.hashCode());
			result = prime * result + ((attributeIdentifiers == null) ? 0 : attributeIdentifiers.hashCode());
			result = prime * result + ((elementIdentifiers == null) ? 0 : elementIdentifiers.hashCode());
			result = prime * result + ((userIdentifiers == null) ? 0 : userIdentifiers.hashCode());
			result = prime * result + ((xmlElementAbstractionMap == null) ? 0 : xmlElementAbstractionMap.hashCode());
			result = prime * result + ((xmlelements == null) ? 0 : xmlelements.hashCode());
			result = prime * result + ((xmls == null) ? 0 : xmls.hashCode());
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
			if (attributeIdentifiers == null) {
				if (other.attributeIdentifiers != null)
					return false;
			} else if (!attributeIdentifiers.equals(other.attributeIdentifiers))
				return false;
			if (elementIdentifiers == null) {
				if (other.elementIdentifiers != null)
					return false;
			} else if (!elementIdentifiers.equals(other.elementIdentifiers))
				return false;
			if (userIdentifiers == null) {
				if (other.userIdentifiers != null)
					return false;
			} else if (!userIdentifiers.equals(other.userIdentifiers))
				return false;
			if (xmlElementAbstractionMap == null) {
				if (other.xmlElementAbstractionMap != null)
					return false;
			} else if (!xmlElementAbstractionMap.equals(other.xmlElementAbstractionMap))
				return false;
			if (xmlelements == null) {
				if (other.xmlelements != null)
					return false;
			} else if (!xmlelements.equals(other.xmlelements))
				return false;
			if (xmls == null) {
				if (other.xmls != null)
					return false;
			} else if (!xmls.equals(other.xmls))
				return false;
			return true;
		}
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new RHFrontendParameterizableTask() {
			@SakerInput(value = { "Configurations" }, required = true)
			public Collection<WildcardPath> configsOption = Collections.emptyList();

			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			@Override
			protected TaskBuilderResult<?> createWorkerTask(TaskContext taskcontext) {
				Set<FileCollectionStrategy> inputcollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					inputcollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				Set<FileCollectionStrategy> configscollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : configsOption) {
					configscollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				return TaskBuilderResult.create(
						TaskIdentifier.builder(XmlParseWorkerTaskFactory.class.getName()).build(),
						new XmlParseWorkerTaskFactory(configscollectionstrategies, inputcollectionstrategies));
			}
		};
	}

}
