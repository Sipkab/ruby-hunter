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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.NavigableSet;
import java.util.TreeMap;
import java.util.TreeSet;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.resource.ResourceCompilerTaskFactory;
import bence.sipka.compiler.types.TypesTaskFactory;
import bence.sipka.compiler.xml.declarations.AttributeDeclaration;
import bence.sipka.compiler.xml.declarations.ElementDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.path.WildcardPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.ParameterizableTask;
import saker.build.task.TaskContext;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.task.utils.annot.SakerInput;
import saker.build.task.utils.dependencies.WildcardFileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class XmlParseTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	private static final String CONFIG_NAMESPACE_URI = "bence.sipka.xml.compiler.config";

	public static final String TASK_NAME = "sipka.rh.xml.parse";

	private static final String ATTR_USER_ID = "id";
	private static final String ATTR_STATIC_TYPE = "staticType";

	private static final String NO_USER_ID_NAME = "NO_ID";
	private static final int NO_USER_ID = -1;

	private static final int FLAG_DYN_TYPE_EMBEDDED = 0x80000000;

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
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {
			@SakerInput(value = { "Configurations" }, required = true)
			public Collection<WildcardPath> configsOption = Collections.emptyList();

			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption = Collections.emptyList();

			private Output output = new Output();

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				Collection<FileCollectionStrategy> inputcollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					inputcollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}
				Collection<FileCollectionStrategy> configscollectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : configsOption) {
					configscollectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, inputcollectionstrategies);
				NavigableMap<SakerPath, SakerFile> configsfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, configscollectionstrategies);

				List<Entry<Document, SakerFile>> configxmldocuments = new ArrayList<>();
				List<Entry<Document, SakerFile>> xmldocuments = new ArrayList<>();

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					Document doc = ResourceCompilerTaskFactory.getDocument(entry.getValue());
					output.xmls.add(entry.getKey());
					xmldocuments.add(ImmutableUtils.makeImmutableMapEntry(doc, entry.getValue()));
				}
				for (Entry<SakerPath, SakerFile> entry : configsfiles.entrySet()) {
					Document doc = ResourceCompilerTaskFactory.getDocument(entry.getValue());
					Node root = doc.getFirstChild();
					if (CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
						configxmldocuments.add(ImmutableUtils.makeImmutableMapEntry(doc, entry.getValue()));
						parseConfigFile(doc);
					}
				}

				output.userIdentifiers.put(NO_USER_ID_NAME, NO_USER_ID);

				for (Entry<String, ElementDeclaration> entry : output.xmlelements.entrySet()) {
					output.xmlElementAbstractionMap.put(entry.getKey(), entry.getValue().isAbstract());
					if (!entry.getValue().isAbstract())
						output.elementIdentifiers.put(entry.getKey(), output.elementIdentifiers.size());
					for (AttributeDeclaration attr : entry.getValue().getAttributes()) {
						String name = attr.getName();
						if (!output.attributeIdentifiers.containsKey(name))
							output.attributeIdentifiers.put(name, output.attributeIdentifiers.size());
					}
				}
				for (Entry<String, ElementDeclaration> entry : output.xmlelements.entrySet()) {
					if (entry.getValue().isAbstract())
						output.elementIdentifiers.put(entry.getKey(), output.elementIdentifiers.size());
				}

				for (Entry<Document, SakerFile> docentry : xmldocuments) {
					parse(docentry.getKey());
				}
				output.userIdentifiers = ImmutableUtils.unmodifiableNavigableMap(output.userIdentifiers);

				SakerPath gendirectorypath = taskcontext.getTaskBuildDirectoryPath()
						.resolve(XmlCompilerTaskFactory.TASK_NAME);
				SakerPath taskworkingdir = taskcontext.getTaskWorkingDirectoryPath();
				for (Entry<Document, SakerFile> docentry : xmldocuments) {
					output.assets.put(taskworkingdir.relativize(docentry.getValue().getSakerPath()).toString(),
							gendirectorypath.resolve(docentry.getValue().getName()));
				}

				return output;
			}

			private ElementDeclaration parseConfigChild(Node n) {
				if (!n.getLocalName().equals(ElementDeclaration.NODE_DECL_ELEM))
					throw new RuntimeException("Invalid node name for xml element declaration: " + n.getLocalName());

				ElementDeclaration decl = new ElementDeclaration(n);
				return decl;
			}

			private void parseConfigFile(Document doc) {
				Node root = doc.getFirstChild();
				NodeList children = root.getChildNodes();
				for (int i = 0; i < children.getLength(); i++) {
					Node item = children.item(i);
					if (item.getNodeType() != Node.ELEMENT_NODE)
						continue;

					ElementDeclaration decl = parseConfigChild(item);
					output.xmlelements.put(decl.getName(), decl);
				}
				doc.removeChild(root);
			}

			private void parse(Document doc) {
				collectIdentifiers(doc.getFirstChild(), output);
			}

		};
	}

	public static void collectIdentifiers(Node n, Output output) {
		if (n == null || n.getNodeType() != Element.ELEMENT_NODE)
			return;
		getUserIdFor(n, output);
		NodeList children = n.getChildNodes();
		for (int i = 0; i < children.getLength(); i++) {
			collectIdentifiers(children.item(i), output);
		}
	}

	public static int getUserIdFor(Node n, Output output) {
		Node idattr = n.getAttributes().getNamedItemNS(CONFIG_NAMESPACE_URI, ATTR_USER_ID);
		if (idattr == null)
			return NO_USER_ID;

		String idname = idattr.getNodeValue();
		int result;
		if (!output.userIdentifiers.containsKey(idname)) {
			result = output.userIdentifiers.size();
			output.userIdentifiers.put(idname, result);
		} else {
			result = output.userIdentifiers.get(idname);
		}
		return result;
	}
}
