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
//package bence.sipka.compiler.resource;
//
//import java.io.IOException;
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Collections;
//import java.util.HashMap;
//import java.util.List;
//import java.util.Map;
//import java.util.Map.Entry;
//import java.util.TreeMap;
//
//import org.w3c.dom.Document;
//import org.w3c.dom.Node;
//import org.w3c.dom.NodeList;
//
//import bence.sipka.compiler.CCompilerOptions;
//import bence.sipka.compiler.source.TemplatedSource;
//import bence.sipka.compiler.source.TemplatedSourceModularFile;
//import bence.sipka.compiler.types.TypeDeclaration;
//import bence.sipka.repository.FrameworkModule;
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import modular.core.data.ModularVariable;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularOutput;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.module.ModuleOperation;
//
//@ModuleURI(ResourceCompiler.MODULE_URI)
//public class ResourceCompiler extends FrameworkModule {
//	public static final String MODULE_URI = "bence.sipka.compiler.resource";
//	private static final String CONFIG_NAMESPACE_URI = "bence.sipka.resource.config";
//
//	private ModularDirectory genDirectory;
//	private ModularDirectory moduleBuildDirectory;
//
//	@ModularData
//	private ModularVariable<Collection<CCompilerOptions>> CGlobalCompilerOptions = ModularVariable
//			.withDefault(ArrayList::new);
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
//		CGlobalCompilerOptions.locked(v -> {
//			v.add(cppoptions);
//		});
//	}
//
//	private static String removeExtension(String s) {
//		int idx = s.lastIndexOf('.');
//		if (idx < 0)
//			return s;
//		return s.substring(0, idx);
//	}
//
//	private static boolean isQualifierDirectory(ModularDirectory dir, Map<String, Qualifier> qualifiers) {
//		String name = dir.getName();
//		int indx = name.indexOf('-');
//		if (indx < 0)
//			return false;
//		return qualifiers.containsKey(name.substring(0, indx));
//	}
//
//	/**
//	 * Represents one resource entry
//	 */
//	private static class QualificationTarget {
//		private String respath;
//		private int resourceId;
//
//		/**
//		 * Maps asset files to qualifier values
//		 */
//		private Map<String, Map<Qualifier, String>> values = new HashMap<>();
//
//		public QualificationTarget(String respath, int resourceId) {
//			this.respath = respath;
//			this.resourceId = resourceId;
//		}
//
//		public void addQualifier(String originalfile, Qualifier qualifier, String value) {
//			// System.out.println("ResourceCompiler.QualificationTarget.addQualifier() " + originalfile + " " + qualifier + " " + value);
//			Map<Qualifier, String> fileq = values.get(originalfile);
//			if (fileq == null) {
//				fileq = new HashMap<>();
//				values.put(originalfile, fileq);
//			}
//			if (fileq.containsKey(qualifier))
//				throw new RuntimeException(
//						"Qualifier applied to file more than once: " + qualifier + " file: " + originalfile);
//			fileq.put(qualifier, value);
//		}
//
//		public void addDefaultQualifier(String originalfile) {
//			// System.out.println("ResourceCompiler.QualificationTarget.addDefaultQualifier() " + originalfile);
//			Map<Qualifier, String> fileq = values.get(originalfile);
//			if (fileq == null) {
//				fileq = Collections.emptyMap();
//				values.put(originalfile, fileq);
//			}
//		}
//
//	}
//
//	@Operation("parse")
//	public class ParseOperation extends ModuleOperation {
//
//		private static final String NODE_QUALIFIERS_ROOT = "Qualifiers";
//		private static final String NODE_QUALIFIER = "qualifier";
//
//		private int residprovider = 0;
//
//		@ModularData
//		private Map<String, TypeDeclaration> typeDeclarations = new HashMap<>();
//
//		@ModularOutput
//		private Map<String, Integer> resourcesmap = new TreeMap<>();
//		@ModularOutput
//		private Map<String, QualificationTarget> resourceTargets = new TreeMap<>();
//
//		@ModularData(required = true)
//		private List<Document> xmldocuments;
//		@ModularData(required = true)
//		private List<ModularFile> xmlfiles;
//
//		@ModularOutput
//		private Map<String, Qualifier> qualifiers = new HashMap<>();
//
//		@ModularData(required = true)
//		private Map<String, Integer> assetsIdentifierMap;
//
//		// @ModularOutput
//		// private Map<PathDelegateModularFile, Collection<String>> qualifiedresources = new HashMap<>();
//		@ModularOutput
//		private Integer qualifiedResourceCount;
//
//		// @ModularOutput(required = true)
//		// private ByteArrayModularFile resourceDictionaryFile;
//
//		@Override
//		public void run() throws Exception {
//			ModularDirectory modulebuilddir = getModuleBuildModularDirectory();
//
//			boolean done = false;
//			for (int i = 0; i < xmldocuments.size(); i++) {
//				Document doc = xmldocuments.get(i);
//				Node root = doc.getFirstChild();
//				if (root != null && CONFIG_NAMESPACE_URI.equals(root.getNamespaceURI())) {
//					if (done)
//						throw new RuntimeException("Only one configuration file is allowed for resource qualifiers");
//
//					parseConfigFile(doc);
//					ModularFile file = xmlfiles.remove(i);
//					file.getParent().hide(file);
//					xmldocuments.remove(i);
//					--i;
//					done = true;
//				}
//			}
//
//			// long[] maxmodified = { getModificationTime() };
//
//			for (Entry<String, Integer> entry : assetsIdentifierMap.entrySet()) {
//				String path = entry.getKey();
//				int dotindex = path.lastIndexOf('.');
//				if (dotindex > path.lastIndexOf('/')) {
//					path = path.substring(0, dotindex);
//				}
//				resourcesmap.put(path, entry.getValue());
//			}
//
//			// TODO uncomment and fix this
//			// projectManager.getRootDirectory().forEachRecursive(f -> f.hasAttribute("application_asset_path"), f -> {
//			// long fmod = f.lastModified();
//			// if (fmod > maxmodified[0]) {
//			// maxmodified[0] = fmod;
//			// }
//			// String path = f.getAttribute("application_asset_path");
//			// int dotindex = path.lastIndexOf('.');
//			// if (dotindex >= 0) {
//			// path = path.substring(0, dotindex);
//			// }
//			// resourcesmap.put(path, Integer.parseUnsignedInt(f.getAttribute("application_asset_identifier"), 16));
//			// });
//
//			// TODO validate, flag eseten egyik nem lehet masik subset-je
//
//			// List<QualificationTarget> singles = new ArrayList<>();
//			// List<QualificationTarget> resvalues = new ArrayList<>(resourceTargets.values());
//			// for (Iterator<QualificationTarget> it = resvalues.iterator(); it.hasNext();) {
//			// QualificationTarget qt = it.next();
//			// if (qt.values.size() == 1) {
//			// singles.add(qt);
//			// it.remove();
//			// }
//			// }
//			// resvalues.sort((o1, o2) -> o1.respath.compareTo(o2.respath));
//			//
//			// for (int i = 0; i < resvalues.size(); i++) {
//			// QualificationTarget qt = resvalues.get(i);
//			// qt.resourceId = i;
//			// resourcesmap.put(qt.respath, qt.resourceId);
//			// }
//			// for (int i = 0; i < singles.size(); i++) {
//			// QualificationTarget qt = singles.get(i);
//			// qt.resourceId = resvalues.size() + i;
//			// // assetsIdentifierMap.put(qt.values.keySet().iterator().next(), qt.resourceId);
//			// resourcesmap.put(qt.respath, qt.resourceId);
//			// }
//
//			// qualifiedResourceCount = resvalues.size();
//			qualifiedResourceCount = 0;
//
//			ResourceReferenceType reftype = new ResourceReferenceType(resourcesmap);
//			typeDeclarations.put(reftype.getName(), reftype);
//
//			// resourceDictionaryFile = new ByteArrayModularFile("RESOURCE_DICTIONARY", getModificationTime());
//
//			// long resourceshmodified = maxmodified[0];
//			//
//			// ModularFile mapfile = modulebuilddir.get("resource_map");
//			// if (mapfile != null) {
//			// Object read = null;
//			// try (ObjectInputStream ois = new ObjectInputStream(mapfile.openInputStream())) {
//			// read = ois.readObject();
//			// } catch (Exception e) {
//			// // ignore
//			// }
//			// if (resourcesmap.equals(read)) {
//			// resourceshmodified = mapfile.lastModified();
//			// }
//			// }
//			// modulebuilddir.add(new SerializeObjectModularFile("resource_map", resourcesmap, resourceshmodified));
//
//			genDirectory.add(new TemplatedSourceModularFile("resources.h",
//					new TemplatedSource(descriptor::getInputStream, "gen/resources.h")).setThis(reftype));
//		}
//
//		private void parseConfigFile(Document doc) {
//			Node root = doc.getFirstChild();
//			if (!root.getLocalName().equals(NODE_QUALIFIERS_ROOT))
//				throw new RuntimeException(
//						"Invalid config root node for resource qualifiers, expected: " + NODE_QUALIFIERS_ROOT);
//
//			NodeList children = root.getChildNodes();
//			int length = children.getLength();
//			for (int i = 0; i < length; i++) {
//				Node child = children.item(i);
//				if (child.getNodeType() != Node.ELEMENT_NODE)
//					continue;
//
//				if (!child.getLocalName().equals(NODE_QUALIFIER))
//					throw new RuntimeException(
//							"Invalid child node for qualifier configuration: " + child.getLocalName());
//
//				Qualifier qual = new Qualifier(child, qualifiers.size(), typeDeclarations.values());
//				if (qualifiers.containsKey(qual.getName())) {
//					throw new RuntimeException("Qualifier already defined with name: " + qual.getName());
//				}
//				qualifiers.put(qual.getName(), qual);
//				// TODO reference qualifiers
//				if (qualifiers.size() > 64)
//					throw new RuntimeException("Maximum of 64 qualifiers are supported");
//			}
//		}
//
//		// private void parseFile(Stack<ModularDirectory> dirstack, Stack<String> appliedQualifiers, ModularFile f) {
//		// ModularDirectory realdir = dirstack.get(dirstack.size() - 1 - appliedQualifiers.size());
//		// String path = resourcesDir.getRelativePath(realdir).replace('\\', '/');
//		// if (!path.equals(""))
//		// path += '/';
//		// String originalpath = path;
//		// String resname = f.getName();
//		// path += removeExtension(resname);
//		//
//		// if (!resourcesmap.containsKey(path)) {
//		// int resid = residprovider++;// resourcesmap.size();
//		// resourceTargets.put(path, new QualificationTarget(path, resid));
//		// resourcesmap.put(path, resid);
//		// }
//		//
//		// Collection<String> contains = qualifiedresources.get(f);
//		// ArrayList<String> nqualifiers = new ArrayList<>(appliedQualifiers);
//		// String newname = realdir.getRelativePath(f).replace('/', '_').replace('\\', '_');
//		// if (contains != null && contains.equals(nqualifiers)) {
//		// throw new RuntimeException("Resources defined twice with same qualifiers: " + nqualifiers);
//		// }
//		// qualifiedresources.put(new PathDelegateModularFile(newname, dirstack.peek(), f), nqualifiers);
//		//
//		// QualificationTarget target = resourceTargets.get(path);
//		// if (nqualifiers.size() > 0) {
//		// for (String q : nqualifiers) {
//		// int separatoridx = q.indexOf('-');
//		// String qname = q.substring(0, separatoridx);
//		// String value = q.substring(separatoridx + 1);
//		// Qualifier qualifier = qualifiers.get(qname);
//		//
//		// target.addQualifier(originalpath + newname, qualifier, value);
//		// }
//		// } else {
//		// target.addDefaultQualifier(originalpath + newname);
//		// }
//		// }
//		//
//		// private void parseDirectory(ModularDirectory dir) {
//		// parseDirectory(new Stack<>(), new Stack<>(), dir);
//		// }
//		//
//		// private void parseDirectory(Stack<ModularDirectory> dirstack, Stack<String> appliedQualifiers, ModularDirectory dir) {
//		// List<ModularFile> files = dir.getFiles();
//		// files.sort((f1, f2) -> Boolean.compare(f2 instanceof ModularDirectory, f1 instanceof ModularDirectory));
//		// dirstack.push(dir);
//		// for (ModularFile f : files) {
//		// if (f instanceof ModularDirectory) {
//		// ModularDirectory fdir = (ModularDirectory) f;
//		// boolean isq = isQualifierDirectory(fdir, qualifiers);
//		// if (!isq && appliedQualifiers.size() > 0) {
//		// throw new RuntimeException("Cannot define non-qualifier directory inside qualified tree: " + fdir);
//		// }
//		// if (isq) {
//		// appliedQualifiers.push(fdir.getName());
//		// }
//		// parseDirectory(dirstack, appliedQualifiers, (ModularDirectory) f);
//		// if (isq) {
//		// appliedQualifiers.pop();
//		// }
//		// } else {
//		// parseFile(dirstack, appliedQualifiers, f);
//		// }
//		// }
//		// dirstack.pop();
//		// }
//	}
//
//	// @Operation("collect")
//	// public class CollectOperation extends ModuleOperation {
//	//
//	// @ModularInput(required = true)
//	// private Map<PathDelegateModularFile, Collection<String>> qualifiedresources = new HashMap<>();
//	//
//	// @ModularInput(required = true, noRemove = true)
//	// private Map<String, Qualifier> qualifiers;
//	//
//	// @Override
//	// public void run() throws Exception {
//	// collectFiles();
//	// }
//	//
//	// private void collectFiles() {
//	// for (Entry<PathDelegateModularFile, Collection<String>> entry : qualifiedresources.entrySet()) {
//	// PathDelegateModularFile f = entry.getKey();
//	// ModularDirectory lastqualifier = null;
//	// ModularDirectory realdir = f.getDelegateDirectory();
//	// while (isQualifierDirectory(realdir, qualifiers)) {
//	// lastqualifier = realdir;
//	// realdir = realdir.getParent();
//	// }
//	//
//	// if (lastqualifier != null)
//	// lastqualifier.getParent().pseudoRemove(lastqualifier);
//	//
//	// if (!f.getDelegateDirectory().equals(realdir)) {
//	// realdir.add(f, ModularFile.FLAG_SHADOWFILE);
//	// }
//	// }
//	// }
//	// }
//
//	// @Operation("qualify")
//	// public class QualifyOperation extends ModuleOperation {
//	//
//	// @ModularInput(required = true)
//	// private ByteArrayModularFile resourceDictionaryFile;
//	// @ModularInput(required = true)
//	// private Map<String, QualificationTarget> resourceTargets;
//	//
//	// @ModularInput(required = true, noRemove = true)
//	// @ModularOutput
//	// private Map<String, Integer> assetsIdentifierMap;
//	//
//	// @ModularInput(required = true)
//	// @ModularOutput
//	// private Map<String, Qualifier> qualifiers;
//	//
//	// // @InputData
//	// // @OutputData
//	// // private List<Object> sourceAdditions = new ArrayList<>();
//	//
//	// @Override
//	// public void run() throws Exception {
//	//
//	// try (ByteArrayOutputStream os = new ByteArrayOutputStream()) {
//	// for (QualificationTarget target : resourceTargets.values()) {
//	// if (target.values.size() <= 1)
//	// continue;
//	//
//	// IntegerType.INSTANCE.serialize(target.resourceId, os);// resource id
//	// IntegerType.INSTANCE.serialize(target.values.size(), os);// assets count
//	//
//	// // System.out.println("resid " + target.resourceId + " assetcount " + target.values.size() + " " + entry.getKey());
//	//
//	// for (Entry<String, Map<Qualifier, String>> valueentry : target.values.entrySet()) {
//	// // TODO order assets id-s by priority
//	// int assetid = assetsIdentifierMap.get(valueentry.getKey());
//	// IntegerType.INSTANCE.serialize(assetid, os); // asset id
//	// IntegerType.INSTANCE.serialize(valueentry.getValue().size(), os); // qualifier count
//	//
//	// // System.out.println("\tassetid " + assetid + " qualifiercount " + valueentry.getValue().size() + " " + valueentry.getKey());
//	//
//	// for (Entry<Qualifier, String> qualifierentry : valueentry.getValue().entrySet()) {
//	// Qualifier qualifier = qualifierentry.getKey();
//	//
//	// // TODO ?
//	// // os.write(qualifier.getTypeDeclaration().getType().ordinal());
//	// os.write(qualifier.getPriority());// qualifier priority
//	// qualifier.getTypeDeclaration().serialize(qualifierentry.getValue(), os);// value for qualifier
//	// }
//	//
//	// }
//	// }
//	//
//	// resourceDictionaryFile.setArray(os.toByteArray());
//	// }
//	//
//	// ArrayList<Qualifier> sorted = new ArrayList<>(qualifiers.values());
//	//
//	// sorted.sort((o1, o2) -> Integer.compare(o1.getPriority(), o2.getPriority()));
//	//
//	// Map<String, Object> valmap = new HashMap<>();
//	// valmap.put("resconfig_constructorparams",
//	// String.join(", ", sorted.stream().map(q -> q.getTypeDeclaration().getTypeRepresentation() + " " + q.getName()).toArray(String[]::new)));
//	// valmap.put("resconfig_constructor_initializer",
//	// String.join(", ", sorted.stream().map(q -> q.getName() + "{ util::move(" + q.getName() + ") }").toArray(String[]::new)));
//	//
//	// // sourceAdditions.add("#include <gen/resconfig.h>");
//	// // sourceAdditions.add("namespace " + CompilerModule.SDK_CPP_NAMESPACE_NAME + " {");
//	// // sourceAdditions.add("ResourceConfiguration ResourceConfiguration::current{0};");
//	// // sourceAdditions.add("}");
//	//
//	// genDirectory.add(new SourceModularFile("resconfig.h",
//	// new TemplatedSource(descriptor::getInputStream, "template_resconfig.h").setValueMap(valmap).setHandler(new TranslationHandler() {
//	// @Override
//	// public void replaceKey(String key, OutputStream os) throws IOException {
//	// PrintStream out = new PrintStream(os);
//	// switch (key) {
//	// case "resconfig_private_members": {
//	// for (Qualifier q : sorted) {
//	// TypeDeclaration type = q.getTypeDeclaration();
//	// out.print(type.getTypeRepresentation() + " " + q.getName() + " { ");
//	// out.print(type.getStringValue(q.getDefaultValue()));
//	// out.println(" };");
//	// }
//	// break;
//	// }
//	// case "resconfig_public_members": {
//	// // for (Qualifier q : sorted) {
//	// // TypeDeclaration type = q.getTypeDeclaration();
//	// // if (type == BuiltinType.STRING) {
//	// // out.println("template<unsigned int n>");
//	// // out.println("void set" + q.getName() + "(const char (&str)[n]){");
//	// // out.println("set" + q.getName() + "(" + type.getTypeRepresentation() + "{str});");
//	// // out.println("}");
//	// // out.println("void set" + q.getName() + "(const char* ptr){");
//	// // out.println("set" + q.getName() + "(" + type.getTypeRepresentation() + "{ptr});");
//	// // out.println("}");
//	// // out.println("void set" + q.getName() + "(" + type.getTypeRepresentation() + " val){");
//	// // out.println(q.getName() + " = util::move(val);");
//	// // out.println("changedMask |= 1ull << (63 - " + q.getPriority() + ");");
//	// // out.println("}");
//	// // } else {
//	// // out.println("void set" + q.getName() + "(const " + type.getTypeRepresentation() + "& val){");
//	// // out.println(q.getName() + " = val;");
//	// // out.println("changedMask |= 1ull << (63 - " + q.getPriority() + ");");
//	// // out.println("}");
//	// // }
//	// // out.println("const " + type.getTypeRepresentation() + "& get" + q.getName() + "() const { return " +
//	// // q.getName() +
//	// // "; }");
//	// // out.println("void set" + q.getName() + "default() {");
//	// // out.println(q.getName() + " = (" + type.getTypeRepresentation() + ") { " +
//	// // type.getStringValue(q.getDefaultValue())
//	// // + "
//	// // };");
//	// // out.println("}");
//	// // }
//	// break;
//	// }
//	// case "resconfig_setatindex": {
//	// // out.println("switch(index){");
//	// // for (Qualifier q : sorted) {
//	// // out.print("case " + q.getPriority() + ": ");
//	// // if (q.getTypeDeclaration() == BuiltinType.STRING) {
//	// // out.print("set" + q.getName() + "({ val.pointer, val.integer });");
//	// // } else if (q.getTypeDeclaration() == BuiltinType.FLOAT) {
//	// // out.print("set" + q.getName() + "({ val.flp });");
//	// // } else {
//	// // out.print("set" + q.getName() + "({ val.integer });");
//	// // }
//	// // out.println(" break;");
//	// // }
//	// // out.println("default: THROW() << \"INVALID INDEX \" << index; break;");
//	// // out.println("}");
//	// break;
//	// }
//	// case "resconfig_matchesatindex": {
//	// // out.println("switch(index){");
//	// // for (Qualifier q : sorted) {
//	// // out.print("case " + q.getPriority() + ": return ");
//	// // if (q.getTypeDeclaration() == BuiltinType.STRING) {
//	// // out.print(" val");
//	// // } else if (q.getTypeDeclaration() == BuiltinType.FLOAT) {
//	// // out.print(" val.flp");
//	// // } else {
//	// // out.print(" val.integer");
//	// // }
//	// // out.println(" == " + q.getName() + ";");
//	// // }
//	// // out.println("default: THROW() << \"INVALID INDEX \" << index; break;");
//	// // out.println("}");
//	// break;
//	// }
//	// case "resconfig_getchangemask": {
//	// for (Qualifier q : sorted) {
//	// out.println(
//	// "if(!(" + q.getName() + " == other." + q.getName() + ")) result |= 1ull << (63 - " + q.getPriority() + ");");
//	// }
//	// break;
//	// }
//	// case "resconfig_matches": {
//	// for (Qualifier q : sorted) {
//	// out.println("if((mask & (1ull << (63 - " + q.getPriority() + "))) && !(" + q.getName() + " == other." + q.getName()
//	// + ")) return false;");
//	// }
//	// break;
//	// }
//	// case "resconfig_initstatic": {
//	// // for (Qualifier q : sorted) {
//	// // out.println("current.set" + q.getName() + "default();");
//	// // }
//	// break;
//	// }
//	// case "resconfig_destroystatic": {
//	// // for (Qualifier q : sorted) {
//	// // TypeDeclaration type = q.getTypeDeclaration();
//	// // if (type == BuiltinType.STRING) {
//	// // out.println("current." + q.getName() + " = (" + type.getTypeRepresentation() + ") { nullptr };");
//	// // }
//	// // }
//	// break;
//	// }
//	// default: {
//	// TranslationHandler.super.replaceKey(key, out);
//	// break;
//	// }
//	// }
//	// }
//	// }), getModificationTime()));
//	// }
//	// }
//
//}
