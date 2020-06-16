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
package bence.sipka.compiler.resource;

import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.TreeMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.asset.AssetsCompilerTaskFactory;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceModularFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
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
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.nest.utils.FrontendTaskFactory;

public class ResourceCompilerTaskFactory extends FrontendTaskFactory<Object> {
	private static final long serialVersionUID = 1L;

	private static final String NODE_QUALIFIERS_ROOT = "Qualifiers";
	private static final String NODE_QUALIFIER = "qualifier";

	private static final String CONFIG_NAMESPACE_URI = "bence.sipka.resource.config";

	public static final String TASK_NAME = "sipka.rh.resources.compile";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("resources");

	public static class Output implements Externalizable {
		private static final long serialVersionUID = 1L;

		private NavigableMap<String, TypeDeclaration> typeDeclarations;
		private SakerPath sourceDirectory;

		/**
		 * For {@link Externalizable}.
		 */
		public Output() {
		}

		public Output(NavigableMap<String, TypeDeclaration> typeDeclarations, SakerPath sourceDirectory) {
			this.sourceDirectory = sourceDirectory;
			this.typeDeclarations = ImmutableUtils.makeImmutableNavigableMap(typeDeclarations);
		}

		public NavigableMap<String, TypeDeclaration> getTypeDeclarations() {
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
	}

	@Override
	public ParameterizableTask<? extends Object> createTask(ExecutionContext executioncontext) {
		return new ParameterizableTask<Object>() {

			@SakerInput(value = { "", "Input" }, required = true)
			public Collection<WildcardPath> inputOption;

			@SakerInput(value = "Assets", required = true)
			public AssetsCompilerTaskFactory.Output assetsInput;

			private Map<String, Qualifier> qualifiers = new HashMap<>();
			private NavigableMap<String, Integer> resourcesmap = new TreeMap<>();
			private NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
			private Integer qualifiedResourceCount;

			@Override
			public Object run(TaskContext taskcontext) throws Exception {
				Collection<FileCollectionStrategy> collectionstrategies = new LinkedHashSet<>();
				for (WildcardPath wc : inputOption) {
					collectionstrategies.add(WildcardFileCollectionStrategy.create(wc));
				}

				SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
						.getDirectoryCreate(TASK_NAME);
				outputDirectory.clear();
				SakerDirectory genDirectory = outputDirectory.getDirectoryCreate("gen");

				NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
						.collectFilesReportInputFileAndAdditionDependency(null, collectionstrategies);

				for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
					Document doc = getDocument(entry.getValue());
					Node root = doc.getFirstChild();
					if (root == null || !CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
						continue;
					}
					parseConfigFile(doc);
				}

				for (Entry<String, Integer> entry : assetsInput.getAssetIdentifiers().entrySet()) {
					String path = entry.getKey();
					int dotindex = path.lastIndexOf('.');
					if (dotindex > path.lastIndexOf('/')) {
						path = path.substring(0, dotindex);
					}
					resourcesmap.put(path, entry.getValue());
				}

				qualifiedResourceCount = 0;

				ResourceReferenceType reftype = new ResourceReferenceType(resourcesmap);
				typeDeclarations.put(reftype.getName(), reftype);

				TemplatedSourceModularFile resourcehfile = new TemplatedSourceModularFile("resources.h",
						new TemplatedSource(descriptor::getInputStream, "gen/resources.h")).setThis(reftype);
				genDirectory.add(resourcehfile);

				taskcontext.getTaskUtilities().reportOutputFileDependency(null,
						SakerPathFiles.toFileContentMap(outputDirectory.getFilesRecursiveByPath(
								outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
				outputDirectory.synchronize();

				return new Output(typeDeclarations, outputDirectory.getSakerPath());
			}

			private void parseConfigFile(Document doc) {
				Node root = doc.getFirstChild();
				if (!root.getLocalName().equals(NODE_QUALIFIERS_ROOT)) {
					throw new RuntimeException(
							"Invalid config root node for resource qualifiers, expected: " + NODE_QUALIFIERS_ROOT);
				}

				NodeList children = root.getChildNodes();
				int length = children.getLength();
				for (int i = 0; i < length; i++) {
					Node child = children.item(i);
					if (child.getNodeType() != Node.ELEMENT_NODE)
						continue;

					if (!child.getLocalName().equals(NODE_QUALIFIER))
						throw new RuntimeException(
								"Invalid child node for qualifier configuration: " + child.getLocalName());

					Qualifier qual = new Qualifier(child, qualifiers.size(), typeDeclarations.values());
					if (qualifiers.containsKey(qual.getName())) {
						throw new RuntimeException("Qualifier already defined with name: " + qual.getName());
					}
					qualifiers.put(qual.getName(), qual);
					// TODO reference qualifiers
					if (qualifiers.size() > 64)
						throw new RuntimeException("Maximum of 64 qualifiers are supported");
				}
			}
		};
	}

	public static Document getDocument(SakerFile file) throws Exception {
		DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
		dbFactory.setNamespaceAware(true);
		DocumentBuilder dBuilder;
		dBuilder = dbFactory.newDocumentBuilder();
		Document doc;
		try (InputStream is = file.openInputStream()) {
			doc = dBuilder.parse(is);
		}

		// about normalization - http://stackoverflow.com/questions/13786607/normalization-in-dom-parsing-with-java-how-does-it-work
		doc.getDocumentElement().normalize();
		return doc;
	}

}
