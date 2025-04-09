package bence.sipka.compiler.resource;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.asset.AssetsCompilerTaskFactory;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class ResourceCompilerWorkerTaskFactory implements TaskFactory<ResourceCompilerTaskFactory.Output>,
		Task<ResourceCompilerTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private static final String NODE_QUALIFIERS_ROOT = "Qualifiers";
	private static final String NODE_QUALIFIER = "qualifier";

	public static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("resources");

	private static final String CONFIG_NAMESPACE_URI = "bence.sipka.resource.config";

	private Set<FileCollectionStrategy> inputOption;

	private AssetsCompilerTaskFactory.Output assetsInput;

	/**
	 * For {@link Externalizable}.
	 */
	public ResourceCompilerWorkerTaskFactory() {
	}

	public ResourceCompilerWorkerTaskFactory(Set<FileCollectionStrategy> inputOption,
			bence.sipka.compiler.asset.AssetsCompilerTaskFactory.Output assetsInput) {
		this.inputOption = inputOption;
		this.assetsInput = assetsInput;
	}

	@Override
	public Task<? extends ResourceCompilerTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public ResourceCompilerTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(ResourceCompilerTaskFactory.TASK_NAME);

		Map<String, Qualifier> qualifiers = new HashMap<>();
		NavigableMap<String, Integer> resourcesmap = new TreeMap<>();
		NavigableMap<String, TypeDeclaration> typeDeclarations = new TreeMap<>();
		Integer qualifiedResourceCount;

		SakerDirectory outputDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(ResourceCompilerTaskFactory.TASK_NAME);
		outputDirectory.clear();
		SakerDirectory genDirectory = outputDirectory.getDirectoryCreate("gen");

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, inputOption);

		for (Entry<SakerPath, SakerFile> entry : inputfiles.entrySet()) {
			Document doc = ResourceCompilerTaskFactory.getDocument(entry.getValue());
			Node root = doc.getFirstChild();
			if (root == null || !CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
				continue;
			}
			parseConfigFile(doc, qualifiers, typeDeclarations);
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

		TemplatedSourceSakerFile resourcehfile = new TemplatedSourceSakerFile("resources.h",
				new TemplatedSource(descriptor::getInputStream, "gen/resources.h")).setThis(reftype);
		genDirectory.add(resourcehfile);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(outputDirectory
				.getFilesRecursiveByPath(outputDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		outputDirectory.synchronize();

		return new ResourceCompilerTaskFactory.Output(typeDeclarations, outputDirectory.getSakerPath());
	}

	private static void parseConfigFile(Document doc, Map<String, Qualifier> qualifiers,
			NavigableMap<String, TypeDeclaration> typeDeclarations) {
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
				throw new RuntimeException("Invalid child node for qualifier configuration: " + child.getLocalName());

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

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, inputOption);
		out.writeObject(assetsInput);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		inputOption = SerialUtils.readExternalImmutableLinkedHashSet(in);
		assetsInput = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((assetsInput == null) ? 0 : assetsInput.hashCode());
		result = prime * result + ((inputOption == null) ? 0 : inputOption.hashCode());
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
		ResourceCompilerWorkerTaskFactory other = (ResourceCompilerWorkerTaskFactory) obj;
		if (assetsInput == null) {
			if (other.assetsInput != null)
				return false;
		} else if (!assetsInput.equals(other.assetsInput))
			return false;
		if (inputOption == null) {
			if (other.inputOption != null)
				return false;
		} else if (!inputOption.equals(other.inputOption))
			return false;
		return true;
	}

}
