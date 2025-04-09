package bence.sipka.compiler.xml;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.List;
import java.util.Map.Entry;
import java.util.NavigableMap;
import java.util.Set;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.resource.ResourceCompilerTaskFactory;
import bence.sipka.compiler.xml.declarations.AttributeDeclaration;
import bence.sipka.compiler.xml.declarations.ElementDeclaration;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.dependencies.FileCollectionStrategy;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.trace.BuildTrace;

public class XmlParseWorkerTaskFactory
		implements TaskFactory<XmlParseTaskFactory.Output>, Task<XmlParseTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	private static final String ATTR_USER_ID = XmlCompilerWorkerTaskFactory.ATTR_USER_ID;

	private static final String NO_USER_ID_NAME = XmlCompilerWorkerTaskFactory.NO_USER_ID_NAME;
	private static final int NO_USER_ID = XmlCompilerWorkerTaskFactory.NO_USER_ID;

	private Set<FileCollectionStrategy> configs;
	private Set<FileCollectionStrategy> input;

	/**
	 * For {@link Externalizable}.
	 */
	public XmlParseWorkerTaskFactory() {
	}

	public XmlParseWorkerTaskFactory(Set<FileCollectionStrategy> configs, Set<FileCollectionStrategy> input) {
		this.configs = configs;
		this.input = input;
	}

	@Override
	public Task<? extends XmlParseTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public XmlParseTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(XmlParseTaskFactory.TASK_NAME);

		XmlParseTaskFactory.Output output = new XmlParseTaskFactory.Output();

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, input);
		NavigableMap<SakerPath, SakerFile> configsfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, configs);

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
			if (XmlCompilerWorkerTaskFactory.CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
				configxmldocuments.add(ImmutableUtils.makeImmutableMapEntry(doc, entry.getValue()));
				parseConfigFile(doc, output);
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
			parse(docentry.getKey(), output);
		}
		output.userIdentifiers = ImmutableUtils.unmodifiableNavigableMap(output.userIdentifiers);

		SakerPath gendirectorypath = taskcontext.getTaskBuildDirectoryPath().resolve(XmlCompilerTaskFactory.TASK_NAME);
		SakerPath taskworkingdir = taskcontext.getTaskWorkingDirectoryPath();
		for (Entry<Document, SakerFile> docentry : xmldocuments) {
			output.assets.put(taskworkingdir.relativize(docentry.getValue().getSakerPath()).toString(),
					gendirectorypath.resolve(docentry.getValue().getName()));
		}

		return output;
	}

	private static ElementDeclaration parseConfigChild(Node n) {
		if (!n.getLocalName().equals(ElementDeclaration.NODE_DECL_ELEM))
			throw new RuntimeException("Invalid node name for xml element declaration: " + n.getLocalName());

		ElementDeclaration decl = new ElementDeclaration(n);
		return decl;
	}

	private static void parseConfigFile(Document doc, XmlParseTaskFactory.Output output) {
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

	private static void parse(Document doc, XmlParseTaskFactory.Output output) {
		collectIdentifiers(doc.getFirstChild(), output);
	}

	public static void collectIdentifiers(Node n, XmlParseTaskFactory.Output output) {
		if (n == null || n.getNodeType() != Element.ELEMENT_NODE)
			return;
		getUserIdFor(n, output);
		NodeList children = n.getChildNodes();
		for (int i = 0; i < children.getLength(); i++) {
			collectIdentifiers(children.item(i), output);
		}
	}

	public static int getUserIdFor(Node n, XmlParseTaskFactory.Output output) {
		Node idattr = n.getAttributes().getNamedItemNS(XmlCompilerWorkerTaskFactory.CONFIG_NAMESPACE_URI, ATTR_USER_ID);
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

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, configs);
		SerialUtils.writeExternalCollection(out, input);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		input = SerialUtils.readExternalImmutableLinkedHashSet(in);
		configs = SerialUtils.readExternalImmutableLinkedHashSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((configs == null) ? 0 : configs.hashCode());
		result = prime * result + ((input == null) ? 0 : input.hashCode());
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
		XmlParseWorkerTaskFactory other = (XmlParseWorkerTaskFactory) obj;
		if (configs == null) {
			if (other.configs != null)
				return false;
		} else if (!configs.equals(other.configs))
			return false;
		if (input == null) {
			if (other.input != null)
				return false;
		} else if (!input.equals(other.input))
			return false;
		return true;
	}

}
