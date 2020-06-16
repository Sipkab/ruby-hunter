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
//package bence.sipka.compiler.xml;
//
//import java.io.ByteArrayOutputStream;
//import java.io.IOException;
//import java.io.OutputStream;
//import java.util.ArrayList;
//import java.util.Arrays;
//import java.util.Collection;
//import java.util.Collections;
//import java.util.HashMap;
//import java.util.List;
//import java.util.Map;
//import java.util.Map.Entry;
//import java.util.TreeMap;
//import java.util.function.Predicate;
//
//import org.w3c.dom.Document;
//import org.w3c.dom.Element;
//import org.w3c.dom.NamedNodeMap;
//import org.w3c.dom.Node;
//import org.w3c.dom.NodeList;
//
//import bence.sipka.compiler.CCompilerOptions;
//import bence.sipka.compiler.CCompilerPass;
//import bence.sipka.compiler.source.SourceTemplateTranslator;
//import bence.sipka.compiler.source.TemplatedSource;
//import bence.sipka.compiler.source.TemplatedSourceModularFile;
//import bence.sipka.compiler.types.TypeDeclaration;
//import bence.sipka.compiler.types.builtin.IntegerType;
//import bence.sipka.compiler.types.builtin.ShortType;
//import bence.sipka.compiler.xml.declarations.AttributeDeclaration;
//import bence.sipka.compiler.xml.declarations.ElementDeclaration;
//import bence.sipka.repository.FrameworkModule;
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularParameter;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.file.content.HashContentDescriptor;
//import modular.core.file.content.FileContentDescriptor;
//import modular.core.module.ModuleOperation;
//
//@ModuleURI(XmlCompiler.MODULE_URI)
//public class XmlCompiler extends FrameworkModule {
//	public static final String MODULE_URI = "bence.sipka.compiler.xml";
//	private static final String CONFIG_NAMESPACE_URI = "bence.sipka.xml.compiler.config";
//
//	private static final String NO_USER_ID_NAME = "NO_ID";
//	private static final int NO_USER_ID = -1;
//
//	private static final int FLAG_DYN_TYPE_EMBEDDED = 0x80000000;
//
//	@ModularData(required = true)
//	private List<Document> xmldocuments;
//	@ModularData(required = true)
//	private List<ModularFile> xmlfiles;
//
//	private ModularDirectory genDirectory;
//	private ModularDirectory moduleBuildDirectory;
//
//	@ModularData
//	private Collection<CCompilerOptions> CGlobalCompilerOptions = new ArrayList<>();
//
//	@Override
//	public void open() throws IOException {
//		super.open();
//		moduleBuildDirectory = getModuleBuildModularDirectory();
//		genDirectory = moduleBuildDirectory.getDirectoryCreate("gen");
//
//		CCompilerOptions cppoptions = CCompilerOptions.create();
//		cppoptions.setLanguage("C++");
//		cppoptions.getIncludeDirectories().add(moduleBuildDirectory.getModularPath().toString());
//		CGlobalCompilerOptions.add(cppoptions);
//	}
//
//	@Operation("parse")
//	public class ParseOperation extends ModuleOperation {
//
//		@ModularData
//		private Map<String, ElementDeclaration> xmlelements = new HashMap<>();
//
//		@Override
//		public void run() throws Exception {
//			for (int i = 0; i < xmldocuments.size(); i++) {
//				Document doc = xmldocuments.get(i);
//				Node root = doc.getFirstChild();
//				if (CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
//					parseConfigFile(doc);
//					ModularFile file = xmlfiles.remove(i);
//					file.getParent().hide(file);
//					xmldocuments.remove(i);
//					--i;
//				}
//			}
//
//		}
//
//		private void parseConfigChild(Node n) {
//			if (!n.getLocalName().equals(ElementDeclaration.NODE_DECL_ELEM))
//				throw new RuntimeException("Invalid node name for xml element declaration: " + n.getLocalName());
//
//			ElementDeclaration decl = new ElementDeclaration(n);
//			xmlelements.put(decl.getName(), decl);
//		}
//
//		private void parseConfigFile(Document doc) {
//			Node root = doc.getFirstChild();
//			NodeList children = root.getChildNodes();
//			for (int i = 0; i < children.getLength(); i++) {
//				Node item = children.item(i);
//				if (item.getNodeType() != Node.ELEMENT_NODE)
//					continue;
//
//				parseConfigChild(item);
//			}
//			doc.removeChild(root);
//		}
//
//	}
//
//	@Operation("translate")
//	public class TranslateOperation extends ModuleOperation {
//
//		private static final String ATTR_USER_ID = "id";
//		private static final String ATTR_STATIC_TYPE = "staticType";
//
//		@ModularData("xmlElementIdentifiers")
//		private Map<String, Integer> elementIdentifiers = new TreeMap<>();
//		@ModularData("xmlAttributeIdentifiers")
//		private Map<String, Integer> attributeIdentifiers = new TreeMap<>();
//		@ModularData("xmlUserIdentifiers")
//		private Map<String, Integer> userIdentifiers = new TreeMap<>();
//
//		@ModularData(required = true)
//		private Map<String, ElementDeclaration> xmlelements;
//
//		@ModularData
//		private Map<String, Boolean> xmlElementAbstractionMap = new HashMap<>();
//
//		@ModularData
//		private Map<String, TypeDeclaration> typeDeclarations = new HashMap<>();
//
//		@ModularParameter
//		private boolean noinflate = false;
//
//		@ModularData
//		private Collection<CCompilerPass> CCompilerPasses = new ArrayList<>();
//
//		private XmlCompileHeaderData xmlCompileHeaderData = new XmlCompileHeaderData();
//
//		@Override
//		public void run() throws Exception {
//			userIdentifiers.put(NO_USER_ID_NAME, NO_USER_ID);
//
//			xmlCompileHeaderData.serializers = new ArrayList<>(typeDeclarations.values());
//			Collections.sort(xmlCompileHeaderData.serializers);
//
//			for (Entry<String, ElementDeclaration> entry : xmlelements.entrySet()) {
//				xmlElementAbstractionMap.put(entry.getKey(), entry.getValue().isAbstract());
//				if (!entry.getValue().isAbstract())
//					elementIdentifiers.put(entry.getKey(), elementIdentifiers.size());
//				for (AttributeDeclaration attr : entry.getValue().getAttributes()) {
//					String name = attr.getName();
//					if (!attributeIdentifiers.containsKey(name))
//						attributeIdentifiers.put(name, attributeIdentifiers.size());
//				}
//			}
//			for (Entry<String, ElementDeclaration> entry : xmlelements.entrySet()) {
//				if (entry.getValue().isAbstract())
//					elementIdentifiers.put(entry.getKey(), elementIdentifiers.size());
//			}
//
//			while (xmldocuments.size() > 0) {
//				Document doc = xmldocuments.remove(0);
//				ModularFile file = xmlfiles.remove(0);
//				ModularDirectory parent = file.getParent();
//				parent.hide(file);
//				translate(doc, file);
//			}
//			userIdentifiers = Collections.unmodifiableMap(userIdentifiers);
//
//			genDirectory.add(new TemplatedSourceModularFile("xmlcompile.h",
//					new TemplatedSource(descriptor::getInputStream, "gen/xmlcompile.h").setThis(xmlCompileHeaderData)));
//			TemplatedSourceModularFile xmlcompilecpp = new TemplatedSourceModularFile("xmlcompile.cpp",
//					new TemplatedSource(descriptor::getInputStream, "gen/xmlcompile.cpp"));
//			genDirectory.add(xmlcompilecpp);
//
//			TreeMap<Integer, String> sortedelements = new TreeMap<>();
//			for (Entry<String, Integer> e : elementIdentifiers.entrySet()) {
//				if (xmlElementAbstractionMap.get(e.getKey()))
//					continue;
//				sortedelements.put(e.getValue(), e.getKey());
//			}
//
//			Map<String, Object> xmldeclhmap = new HashMap<>();
//			xmldeclhmap.put("attributes", attributeIdentifiers);
//			xmldeclhmap.put("elements", elementIdentifiers);
//			xmldeclhmap.put("identifiers", userIdentifiers);
//			xmldeclhmap.put("inflate_lut", sortedelements);
//			xmldeclhmap.put("noinflate", noinflate);
//
//			genDirectory.add(new TemplatedSourceModularFile("xmldecl.h",
//					new TemplatedSource(descriptor::getInputStream, "gen/xmldecl.h").setValueMap(xmldeclhmap)));
//			TemplatedSourceModularFile xmldeclcpp = new TemplatedSourceModularFile("xmldecl.cpp",
//					new TemplatedSource(descriptor::getInputStream, "gen/xmldecl.cpp").setValueMap(xmldeclhmap));
//			genDirectory.add(xmldeclcpp);
//
//			CCompilerPass cpppass = CCompilerPass.create();
//			cpppass.setLanguage("C++");
//			cpppass.getFiles().add(xmldeclcpp.getModularPath().toString());
//			cpppass.getFiles().add(xmlcompilecpp.getModularPath().toString());
//			CCompilerPasses.add(cpppass);
//		}
//
//		public String toCppName(String name) {
//			if (Arrays.binarySearch(SourceTemplateTranslator.CPP_KEYWORDS, name) >= 0) {
//				return name + "_";
//			}
//			return name;
//		}
//
//		private void translate(Document doc, ModularFile file) {
//			file.getParent().add(new ModularFile(file.getName()) {
//				@Override
//				public FileContentDescriptor getContentDescriptor() {
//					return new HashContentDescriptor(this);
//				}
//
//				@Override
//				public void writeToStream(OutputStream os) throws IOException {
//					// version
//					IntegerType.INSTANCE.serialize(1, os);
//
//					TranslateMetaData meta = new TranslateMetaData();
//					try (ByteArrayOutputStream rootos = new ByteArrayOutputStream()) {
//						translate(doc.getFirstChild(), rootos, meta);
//
//						// max attribute count for element
//						IntegerType.INSTANCE.serialize(meta.maxAttributeCount, os);
//						// size of the largest attribute block
//						IntegerType.INSTANCE.serialize(meta.maxAttributeBlockLength, os);
//
//						rootos.writeTo(os);
//					}
//				}
//
//			}, ModularFile.FLAG_SHADOWFILE);
//			collectIdentifiers(doc.getFirstChild());
//		}
//
//		private void collectIdentifiers(Node n) {
//			if (n == null || n.getNodeType() != Element.ELEMENT_NODE)
//				return;
//			getUserIdFor(n);
//			NodeList children = n.getChildNodes();
//			for (int i = 0; i < children.getLength(); i++) {
//				collectIdentifiers(children.item(i));
//			}
//		}
//
//		private void translate(Node n, OutputStream os, TranslateMetaData meta) throws IOException {
//			if (n.getNodeType() != Element.ELEMENT_NODE)
//				return;
//
//			NamedNodeMap attrs = n.getAttributes();
//			NodeList children = n.getChildNodes();
//
//			String nodename = n.getLocalName();
//			String searchname = isEmbedded(nodename) ? nodename.substring(nodename.indexOf('.') + 1) : nodename;
//			ElementDeclaration dynamictype = xmlelements.get(searchname);
//			if (dynamictype == null)
//				throw new RuntimeException("Failed to find declared element with name: " + searchname);
//			ElementDeclaration statictype = getStaticTypeFor(n, dynamictype);
//
//			final int childcount = countValidChildren(children);
//			final int attributecount = countValidAttributes(attrs);
//
//			IntegerType.INSTANCE
//					.serialize(getIdentifier(dynamictype) | (isEmbedded(nodename) ? FLAG_DYN_TYPE_EMBEDDED : 0), os);
//			IntegerType.INSTANCE.serialize(getIdentifier(statictype), os);
//			IntegerType.INSTANCE.serialize(getUserIdFor(n), os);
//			IntegerType.INSTANCE.serialize(childcount, os);
//			IntegerType.INSTANCE.serialize(attributecount, os);
//
//			try (ByteArrayOutputStream attributeblock = new ByteArrayOutputStream();
//					ByteArrayOutputStream attrvaluebuf = new ByteArrayOutputStream()) {
//				final int attributelength = attrs.getLength();
//				for (int i = 0; i < attributelength; i++) {
//					Node attr = attrs.item(i);
//					if (!attributeTester.test(attr))
//						continue;
//					AttributeDeclaration attrdecl = dynamictype.findAttribute(attr.getLocalName(), xmlelements);
//					if (attrdecl == null)
//						throw new RuntimeException("Attribute declaration: " + attr.getLocalName()
//								+ " not found for element: " + searchname);
//
//					TypeDeclaration type = parseType(attrdecl.getType());
//					if (type == null) {
//						throw new RuntimeException(
//								"Type not found for name: " + attrdecl.getType() + " for node: " + n.getNodeName());
//					}
//					short id = (short) xmlCompileHeaderData.serializers.indexOf(type);
//
//					// buffer attr value
//					attrvaluebuf.reset();
//					type.serialize(attr.getNodeValue(), attrvaluebuf);
//
//					// attribute id
//					IntegerType.INSTANCE.serialize(getIdentifier(attrdecl), attributeblock);
//					// attribute type id
//					ShortType.INSTANCE.serialize(id, attributeblock);
//					// attribute value length
//					IntegerType.INSTANCE.serialize(attrvaluebuf.size(), attributeblock);
//					// attribute value
//					attrvaluebuf.writeTo(attributeblock);
//				}
//				// attribute block length
//				IntegerType.INSTANCE.serialize(attributeblock.size(), os);
//				attributeblock.writeTo(os);
//
//				meta.maxAttributeCount = Math.max(meta.maxAttributeCount, attributecount);
//				meta.maxAttributeBlockLength = Math.max(meta.maxAttributeBlockLength, attributeblock.size());
//			}
//			for (int i = 0; i < children.getLength(); i++) {
//				translate(children.item(i), os, meta);
//			}
//		}
//
//		private int getIdentifier(ElementDeclaration d) {
//			return elementIdentifiers.get(d.getName());
//		}
//
//		private int getIdentifier(AttributeDeclaration d) {
//			return attributeIdentifiers.get(d.getName());
//		}
//
//		private boolean isEmbedded(String elememtname) {
//			return elememtname.indexOf('.') >= 0;
//		}
//
//		private Predicate<Node> attributeTester = attr -> {
//			return !"xmlns".equals(attr.getPrefix()) && !CONFIG_NAMESPACE_URI.equals(attr.getNamespaceURI());
//		};
//
//		private int countValidAttributes(NamedNodeMap attrs) {
//			int result = 0;
//			for (int i = 0; i < attrs.getLength(); i++) {
//				if (attributeTester.test(attrs.item(i)))
//					++result;
//			}
//			return result;
//		}
//
//		private int countValidChildren(NodeList children) {
//			int count = 0;
//			for (int i = 0; i < children.getLength(); i++) {
//				if (children.item(i).getNodeType() != Node.ELEMENT_NODE)
//					continue;
//				++count;
//			}
//			return count;
//		}
//
//		private int getUserIdFor(Node n) {
//			Node idattr = n.getAttributes().getNamedItemNS(CONFIG_NAMESPACE_URI, ATTR_USER_ID);
//			if (idattr == null)
//				return NO_USER_ID;
//
//			String idname = idattr.getNodeValue();
//			int result;
//			if (!userIdentifiers.containsKey(idname)) {
//				result = userIdentifiers.size();
//				userIdentifiers.put(idname, result);
//			} else {
//				result = userIdentifiers.get(idname);
//			}
//			return result;
//		}
//
//		private ElementDeclaration getStaticTypeFor(Node n, ElementDeclaration dynamictype) {
//			Node staticattr = n.getAttributes().getNamedItem(ATTR_STATIC_TYPE);
//			if (staticattr != null) {
//				ElementDeclaration found = xmlelements.get(staticattr.getNodeValue());
//				if (found == null)
//					throw new RuntimeException("Type not found for static type: " + staticattr.getNodeValue());
//
//				if (!dynamictype.isInstanceOf(found, xmlelements))
//					throw new RuntimeException("Type: " + dynamictype.getName() + " does not inherit from: "
//							+ found.getName() + " in static type declaration");
//
//				return found;
//			}
//			if (dynamictype.getDefaultStaticType() == null) {
//				return dynamictype;
//			}
//			return xmlelements.get(dynamictype.getDefaultStaticType());
//		}
//
//		public TypeDeclaration parseType(String name) {
//			return typeDeclarations.get(name);
//		}
//	}
//}
