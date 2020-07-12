package bence.sipka.compiler.xml;

import java.io.ByteArrayOutputStream;
import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.function.Predicate;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.resource.ResourceCompilerTaskFactory;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.TypesTaskFactory;
import bence.sipka.compiler.types.builtin.IntegerType;
import bence.sipka.compiler.types.builtin.ShortType;
import bence.sipka.compiler.xml.XmlCompilerTaskFactory.Output;
import bence.sipka.compiler.xml.declarations.AttributeDeclaration;
import bence.sipka.compiler.xml.declarations.ElementDeclaration;
import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.file.ByteArraySakerFile;
import saker.build.file.DirectoryVisitPredicate;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.SakerPathFiles;
import saker.build.runtime.execution.ExecutionContext;
import saker.build.task.Task;
import saker.build.task.TaskContext;
import saker.build.task.TaskFactory;
import saker.build.task.utils.dependencies.EqualityTaskOutputChangeDetector;
import saker.build.thirdparty.saker.util.ImmutableUtils;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;
import saker.build.trace.BuildTrace;

public class XmlCompilerWorkerTaskFactory
		implements TaskFactory<XmlCompilerTaskFactory.Output>, Task<XmlCompilerTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final String CONFIG_NAMESPACE_URI = "bence.sipka.xml.compiler.config";

	public static final String ATTR_USER_ID = "id";
	public static final String ATTR_STATIC_TYPE = "staticType";

	public static final String NO_USER_ID_NAME = "NO_ID";
	public static final int NO_USER_ID = -1;

	public static final int FLAG_DYN_TYPE_EMBEDDED = 0x80000000;

	public static final BundleResourceSupplier descriptor = BundleContentAccess
			.getBundleResourceSupplier("xml_compiler");

	private XmlParseTaskFactory.Output output;
	private TypesTaskFactory.Output typesOption;

	/**
	 * For {@link Externalizable}.
	 */
	public XmlCompilerWorkerTaskFactory() {
	}

	public XmlCompilerWorkerTaskFactory(bence.sipka.compiler.xml.XmlParseTaskFactory.Output output,
			bence.sipka.compiler.types.TypesTaskFactory.Output typesOption) {
		this.output = output;
		this.typesOption = typesOption;
	}

	@Override
	public Task<? extends Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
		}
		taskcontext.setStandardOutDisplayIdentifier(XmlCompilerTaskFactory.TASK_NAME);

		boolean noinflate = false;

		SakerDirectory buildDirectory = SakerPathFiles.requireBuildDirectory(taskcontext)
				.getDirectoryCreate(XmlCompilerTaskFactory.TASK_NAME);
		buildDirectory.clear();

		SakerDirectory genDirectory = buildDirectory.getDirectoryCreate("gen");

		List<Entry<Document, SakerFile>> xmldocuments = new ArrayList<>();

		for (SakerPath path : output.xmls) {
			SakerFile f = taskcontext.getTaskUtilities().resolveFileAtPath(path);
			Document doc = ResourceCompilerTaskFactory.getDocument(f);
			xmldocuments.add(ImmutableUtils.makeImmutableMapEntry(doc, f));

			taskcontext.getTaskUtilities().reportInputFileDependency(null, f);
		}

		XmlCompileHeaderData xmlCompileHeaderData = new XmlCompileHeaderData();

		xmlCompileHeaderData.serializers = new ArrayList<>(typesOption.getTypeDeclarations().values());
		Collections.sort(xmlCompileHeaderData.serializers);

		for (Entry<Document, SakerFile> docentry : xmldocuments) {
			ByteArrayRegion compiledbytes = compile(docentry.getKey(), xmlCompileHeaderData);
			ByteArraySakerFile nfile = new ByteArraySakerFile(docentry.getValue().getName(), compiledbytes);
			SakerFile prev = buildDirectory.add(nfile);
			if (prev != null) {
				throw new IllegalArgumentException("Duplicate xmls: " + docentry.getValue().getName());
			}
		}

		genDirectory.add(new TemplatedSourceSakerFile("xmlcompile.h",
				new TemplatedSource(descriptor::getInputStream, "gen/xmlcompile.h").setThis(xmlCompileHeaderData)));
		TemplatedSourceSakerFile xmlcompilecpp = new TemplatedSourceSakerFile("xmlcompile.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/xmlcompile.cpp"));
		genDirectory.add(xmlcompilecpp);

		TreeMap<Integer, String> sortedelements = new TreeMap<>();
		for (Entry<String, Integer> e : output.elementIdentifiers.entrySet()) {
			if (output.xmlElementAbstractionMap.get(e.getKey()))
				continue;
			sortedelements.put(e.getValue(), e.getKey());
		}

		Map<String, Object> xmldeclhmap = new HashMap<>();
		xmldeclhmap.put("attributes", output.attributeIdentifiers);
		xmldeclhmap.put("elements", output.elementIdentifiers);
		xmldeclhmap.put("identifiers", output.userIdentifiers);
		xmldeclhmap.put("inflate_lut", sortedelements);
		xmldeclhmap.put("noinflate", noinflate);

		genDirectory.add(new TemplatedSourceSakerFile("xmldecl.h",
				new TemplatedSource(descriptor::getInputStream, "gen/xmldecl.h").setValueMap(xmldeclhmap)));
		TemplatedSourceSakerFile xmldeclcpp = new TemplatedSourceSakerFile("xmldecl.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/xmldecl.cpp").setValueMap(xmldeclhmap));
		genDirectory.add(xmldeclcpp);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(buildDirectory
				.getFilesRecursiveByPath(buildDirectory.getSakerPath(), DirectoryVisitPredicate.everything())));
		buildDirectory.synchronize();

		Output result = new Output(buildDirectory.getSakerPath());
		taskcontext.reportSelfTaskOutputChangeDetector(new EqualityTaskOutputChangeDetector(result));
		return result;
	}

	private ByteArrayRegion compile(Document doc, XmlCompileHeaderData xmlCompileHeaderData) {
		try (UnsyncByteArrayOutputStream os = new UnsyncByteArrayOutputStream()) {
			// version
			IntegerType.INSTANCE.serialize(1, os);

			TranslateMetaData meta = new TranslateMetaData();
			try (ByteArrayOutputStream rootos = new ByteArrayOutputStream()) {
				compile(doc.getFirstChild(), rootos, meta, xmlCompileHeaderData);

				// max attribute count for element
				IntegerType.INSTANCE.serialize(meta.maxAttributeCount, os);
				// size of the largest attribute block
				IntegerType.INSTANCE.serialize(meta.maxAttributeBlockLength, os);

				rootos.writeTo(os);
				return os.toByteArrayRegion();
			}
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	private void compile(Node n, OutputStream os, TranslateMetaData meta, XmlCompileHeaderData xmlCompileHeaderData)
			throws IOException {
		if (n.getNodeType() != Element.ELEMENT_NODE)
			return;

		NamedNodeMap attrs = n.getAttributes();
		NodeList children = n.getChildNodes();

		String nodename = n.getLocalName();
		String searchname = isEmbedded(nodename) ? nodename.substring(nodename.indexOf('.') + 1) : nodename;
		ElementDeclaration dynamictype = output.xmlelements.get(searchname);
		if (dynamictype == null)
			throw new RuntimeException("Failed to find declared element with name: " + searchname);
		ElementDeclaration statictype = getStaticTypeFor(n, dynamictype);

		final int childcount = countValidChildren(children);
		final int attributecount = countValidAttributes(attrs);

		IntegerType.INSTANCE.serialize(getIdentifier(dynamictype) | (isEmbedded(nodename) ? FLAG_DYN_TYPE_EMBEDDED : 0),
				os);
		IntegerType.INSTANCE.serialize(getIdentifier(statictype), os);
		IntegerType.INSTANCE.serialize(XmlParseWorkerTaskFactory.getUserIdFor(n, output), os);
		IntegerType.INSTANCE.serialize(childcount, os);
		IntegerType.INSTANCE.serialize(attributecount, os);

		try (ByteArrayOutputStream attributeblock = new ByteArrayOutputStream();
				ByteArrayOutputStream attrvaluebuf = new ByteArrayOutputStream()) {
			final int attributelength = attrs.getLength();
			for (int i = 0; i < attributelength; i++) {
				Node attr = attrs.item(i);
				if (!attributeTester.test(attr))
					continue;
				AttributeDeclaration attrdecl = dynamictype.findAttribute(attr.getLocalName(), output.xmlelements);
				if (attrdecl == null)
					throw new RuntimeException(
							"Attribute declaration: " + attr.getLocalName() + " not found for element: " + searchname);

				TypeDeclaration type = parseType(attrdecl.getType());
				if (type == null) {
					throw new RuntimeException(
							"Type not found for name: " + attrdecl.getType() + " for node: " + n.getNodeName());
				}
				short id = (short) xmlCompileHeaderData.serializers.indexOf(type);

				// buffer attr value
				attrvaluebuf.reset();
				type.serialize(attr.getNodeValue(), attrvaluebuf);

				// attribute id
				IntegerType.INSTANCE.serialize(getIdentifier(attrdecl), attributeblock);
				// attribute type id
				ShortType.INSTANCE.serialize(id, attributeblock);
				// attribute value length
				IntegerType.INSTANCE.serialize(attrvaluebuf.size(), attributeblock);
				// attribute value
				attrvaluebuf.writeTo(attributeblock);
			}
			// attribute block length
			IntegerType.INSTANCE.serialize(attributeblock.size(), os);
			attributeblock.writeTo(os);

			meta.maxAttributeCount = Math.max(meta.maxAttributeCount, attributecount);
			meta.maxAttributeBlockLength = Math.max(meta.maxAttributeBlockLength, attributeblock.size());
		}
		for (int i = 0; i < children.getLength(); i++) {
			compile(children.item(i), os, meta, xmlCompileHeaderData);
		}
	}

	private int getIdentifier(ElementDeclaration d) {
		return output.elementIdentifiers.get(d.getName());
	}

	private int getIdentifier(AttributeDeclaration d) {
		return output.attributeIdentifiers.get(d.getName());
	}

	private static boolean isEmbedded(String elememtname) {
		return elememtname.indexOf('.') >= 0;
	}

	private Predicate<Node> attributeTester = attr -> {
		return !"xmlns".equals(attr.getPrefix()) && !CONFIG_NAMESPACE_URI.equals(attr.getNamespaceURI());
	};

	private int countValidAttributes(NamedNodeMap attrs) {
		int result = 0;
		for (int i = 0; i < attrs.getLength(); i++) {
			if (attributeTester.test(attrs.item(i)))
				++result;
		}
		return result;
	}

	private static int countValidChildren(NodeList children) {
		int count = 0;
		for (int i = 0; i < children.getLength(); i++) {
			if (children.item(i).getNodeType() != Node.ELEMENT_NODE)
				continue;
			++count;
		}
		return count;
	}

	private ElementDeclaration getStaticTypeFor(Node n, ElementDeclaration dynamictype) {
		Node staticattr = n.getAttributes().getNamedItem(ATTR_STATIC_TYPE);
		if (staticattr != null) {
			ElementDeclaration found = output.xmlelements.get(staticattr.getNodeValue());
			if (found == null)
				throw new RuntimeException("Type not found for static type: " + staticattr.getNodeValue());

			if (!dynamictype.isInstanceOf(found, output.xmlelements))
				throw new RuntimeException("Type: " + dynamictype.getName() + " does not inherit from: "
						+ found.getName() + " in static type declaration");

			return found;
		}
		if (dynamictype.getDefaultStaticType() == null) {
			return dynamictype;
		}
		return output.xmlelements.get(dynamictype.getDefaultStaticType());
	}

	public TypeDeclaration parseType(String name) {
		return typesOption.getTypeDeclarations().get(name);
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(output);
		out.writeObject(typesOption);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		output = SerialUtils.readExternalObject(in);
		typesOption = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((attributeTester == null) ? 0 : attributeTester.hashCode());
		result = prime * result + ((output == null) ? 0 : output.hashCode());
		result = prime * result + ((typesOption == null) ? 0 : typesOption.hashCode());
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
		XmlCompilerWorkerTaskFactory other = (XmlCompilerWorkerTaskFactory) obj;
		if (attributeTester == null) {
			if (other.attributeTester != null)
				return false;
		} else if (!attributeTester.equals(other.attributeTester))
			return false;
		if (output == null) {
			if (other.output != null)
				return false;
		} else if (!output.equals(other.output))
			return false;
		if (typesOption == null) {
			if (other.typesOption != null)
				return false;
		} else if (!typesOption.equals(other.typesOption))
			return false;
		return true;
	}

}
