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
//package bence.sipka.compiler.shader;
//
//import java.io.IOException;
//import java.io.InputStream;
//import java.io.OutputStream;
//import java.io.PrintStream;
//import java.util.ArrayList;
//import java.util.Collection;
//import java.util.Collections;
//import java.util.HashMap;
//import java.util.List;
//import java.util.Map;
//import java.util.function.Supplier;
//import java.util.stream.Collectors;
//
//import bence.sipka.compiler.CCompilerOptions;
//import bence.sipka.compiler.CCompilerPass;
//import bence.sipka.compiler.shader.elements.VariableDeclaration;
//import bence.sipka.compiler.shader.translator.DirectX11ShaderTranslator;
//import bence.sipka.compiler.shader.translator.OpenGl30ShaderTranslator;
//import bence.sipka.compiler.shader.translator.OpenglEs20ShaderTranslator;
//import bence.sipka.compiler.shader.translator.ShaderTranslator;
//import bence.sipka.compiler.shader.translator2.DirectX11ShaderTranslator2;
//import bence.sipka.compiler.shader.translator2.OpenGlEsShaderTranslator2;
//import bence.sipka.compiler.shader.translator2.OpenGlShaderTranslator2;
//import bence.sipka.compiler.shader.translator2.ShaderTranslator2;
//import bence.sipka.compiler.shader.translator2.program.ShaderEnvironment;
//import bence.sipka.compiler.source.SourceTemplateTranslator.TranslationHandler;
//import bence.sipka.compiler.source.TemplatedSource;
//import bence.sipka.compiler.source.TemplatedSourceModularFile;
//import bence.sipka.compiler.types.enums.EnumType;
//import bence.sipka.repository.FrameworkModule;
//import bence.sipka.repository.annot.ModuleURI;
//import bence.sipka.repository.annot.Operation;
//import bence.sipka.utils.BundleContentAccess;
//import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
//import modular.core.data.ModularVariable;
//import modular.core.data.annotation.ModularData;
//import modular.core.data.annotation.ModularParameter;
//import modular.core.file.ModularDirectory;
//import modular.core.file.ModularFile;
//import modular.core.module.ModuleOperation;
//import saker.build.file.path.WildcardPath;
//import saker.build.thirdparty.saker.util.thread.ThreadUtils;
//import sipka.syntax.parser.model.ParseFailedException;
//import sipka.syntax.parser.model.rule.Language;
//import sipka.syntax.parser.model.statement.Statement;
//
//@ModuleURI(ShaderCompiler.MODULE_URI)
//public class ShaderCompiler extends FrameworkModule {
//	public static final String MODULE_URI = "bence.sipka.compiler.shaders";
//
//	private static final BundleResourceSupplier descriptor = BundleContentAccess
//			.getBundleResourceSupplier("shader_translator");
//
//	private static final Map<String, Supplier<ShaderTranslator>> TRANSLATOR_MAP = new HashMap<>();
//	private static final Map<String, Supplier<ShaderTranslator2>> TRANSLATOR2_MAP = new HashMap<>();
//	static {
//		TRANSLATOR_MAP.put("opengles20", OpenglEs20ShaderTranslator::new);
//		TRANSLATOR_MAP.put("opengl30", OpenGl30ShaderTranslator::new);
//		TRANSLATOR_MAP.put("directx11", DirectX11ShaderTranslator::new);
//
//		TRANSLATOR2_MAP.put("opengles20", OpenGlEsShaderTranslator2::new);
//		TRANSLATOR2_MAP.put("opengl30", OpenGlShaderTranslator2::new);
//		TRANSLATOR2_MAP.put("directx11", DirectX11ShaderTranslator2::new);
//	}
//
//	private static Language UNIFIED_SHADER_LANG;
//
//	private synchronized Language getUnifiedShaderLanguage() throws IOException {
//		if (UNIFIED_SHADER_LANG == null) {
//			try (InputStream is = descriptor.getInputStream("unisl.lang")) {
//				UNIFIED_SHADER_LANG = Language.fromInputStream(is).get("unified_shading_lang");
//			} catch (ParseFailedException e) {
//				throw new IOException(e);
//			}
//		}
//		return UNIFIED_SHADER_LANG;
//	}
//
//	private ModularDirectory buildDirectory;
//	private ModularDirectory sourcesDirectory;
//	private ModularDirectory shaderGenSourcesDir;
//	private ModularDirectory shadersDir;
//
//	@ModularData
//	private ModularVariable<Collection<CCompilerOptions>> CGlobalCompilerOptions = ModularVariable
//			.withDefault(ArrayList::new);
//	@ModularData
//	private ModularVariable<Collection<CCompilerPass>> CCompilerPasses = ModularVariable.withDefault(ArrayList::new);
//
//	@ModularData
//	private Collection<WildcardPath> ApplicationAssets = new ArrayList<>();
//
//	@Override
//	public void open() throws IOException {
//		super.open();
//		buildDirectory = getModuleBuildModularDirectory();
//		sourcesDirectory = buildDirectory.getDirectoryCreate("sources");
//		shadersDir = buildDirectory.getDirectoryCreate("shaders");
//		CCompilerOptions cppoptions = CCompilerOptions.create();
//		cppoptions.setLanguage("C++");
//		cppoptions.getIncludeDirectories().add(sourcesDirectory.getSakerPath().toString());
//		CGlobalCompilerOptions.locked(v -> {
//			v.add(cppoptions);
//		});
//
//		shaderGenSourcesDir = sourcesDirectory.getDirectoryCreate("gen").getDirectoryCreate("shader");
//	}
//
//	@Operation("translate")
//	public class TranslateOperation extends ModuleOperation {
//
//		private ShaderCollection shaders = new ShaderCollection();
//		private Collection<ModularFile> toRemove = new ArrayList<>();
//
//		@ModularParameter(required = true)
//		private List<String> renderapi;
//
//		@ModularParameter
//		private Collection<WildcardPath> UniversalShaders = Collections.emptyList();
//
//		@ModularData
//		private Map<String, bence.sipka.compiler.types.TypeDeclaration> typeDeclarations = new HashMap<>();
//
//		private ShaderEnvironment shaderEnv = new ShaderEnvironment();
//
//		@Override
//		public void run() throws Exception {
//			for (ModularFile file : WildcardPath.getFiles(getContext(), UniversalShaders)) {
//				toRemove.add(file);
//			}
//
//			ThreadUtils.runParallel(toRemove, f -> {
//				f.getParent().hide(f);
//				// f.getParent().pseudoRemove(f);
//
//				try {
//					Statement parsed;
//					try (InputStream is = f.openInputStream()) {
//						synchronized (this) {
//							parsed = getUnifiedShaderLanguage().parseInputStream(is).getStatement();
//						}
//					}
//					for (Statement shadernodes : parsed.scopeTo("shader_node")) {
//						synchronized (shaderEnv) {
//							shaderEnv.addShader(shadernodes);
//						}
//						synchronized (shaders) {
//							ShaderProgram program = new ShaderProgram(shadernodes, shaders);
//							program.setFile(f);
//							shaders.addProgram(program);
//						}
//					}
//				} catch (ParseFailedException | IOException e) {
//					throw new RuntimeException("Error in file: " + f.getName(), e);
//				}
//			});
//
//			// TODO itt a translatorok az enum ertekuk szerint legyenek rendezve
//			Collection<ShaderTranslator> translators = renderapi.stream().map(api -> TRANSLATOR_MAP.get(api).get())
//					.sorted((a, b) -> a.getUniqueName().compareTo(b.getUniqueName())).collect(Collectors.toList());
//
//			Collection<ShaderTranslator2> translators2 = renderapi.stream().map(api -> TRANSLATOR2_MAP.get(api).get())
//					.sorted((a, b) -> a.getUniqueName().compareTo(b.getUniqueName())).collect(Collectors.toList());
//
//			translateShaders();
//
//			Map<String, Object> shadersmap = new HashMap<>();
//			shadersmap.put("rendererconfigs",
//					translators.stream().map(r -> r.getUniqueName()).collect(Collectors.toCollection(ArrayList::new)));
//			shadersmap.put("programnames", shaders.getPrograms().stream().map(p -> p.getName())
//					.collect(Collectors.toCollection(ArrayList::new)));
//			shadersmap.put("shaderclassnames", shaders.getShaders().stream()
//					.map(s -> s.getClassUrl().getExactClassName()).collect(Collectors.toCollection(ArrayList::new)));
//
//			shaderGenSourcesDir.add(new TemplatedSourceModularFile("shaders.h",
//					new TemplatedSource(descriptor::getInputStream, "gen/shader/shaders.h")));
//			TemplatedSourceModularFile shaderscpp = new TemplatedSourceModularFile("shaders.cpp",
//					new TemplatedSource(descriptor::getInputStream, "gen/shader/shaders.cpp")).setValueMap(shadersmap);
//			shaderGenSourcesDir.add(shaderscpp);
//
//			CCompilerPass cpppass = CCompilerPass.create();
//			cpppass.setLanguage("C++");
//			cpppass.getFiles().add(shaderscpp.getSakerPath().toString());
//			CCompilerPasses.locked(v -> {
//				v.add(cpppass);
//			});
//
//			for (ShaderTranslator translator : translators) {
//				log().println("ShaderCompiler.TranslateOperation.run() " + translator.getUniqueName());
//				String uniquename = translator.getUniqueName().toLowerCase();
//				ModularDirectory translatordir = shaderGenSourcesDir.getDirectoryCreate(uniquename);
//				ModularDirectory shadersourcesdir = shadersDir.getDirectoryCreate(uniquename);
//				translator.translate(ShaderCompiler.this, translatordir, shadersourcesdir, shaders, ApplicationAssets);
//				log().println("ShaderCompiler.TranslateOperation.run() " + translator.getUniqueName() + " end");
//			}
//
//			EnumType unifiedshaders = new EnumType("UnifiedShaders");
//			EnumType pipelines = new EnumType("UnifiedShaderPipelineStage");
//			int counter;
//			counter = 0;
//			for (ShaderProgram prog : shaders.getPrograms()) {
//				unifiedshaders.add(prog.getName(), counter++);
//			}
//			counter = 0;
//			for (ShaderResource shader : shaders.getShaders()) {
//				pipelines.add(shader.getClassUrl().getExactClassName(), counter++);
//			}
//
//			typeDeclarations.put(unifiedshaders.getName(), unifiedshaders);
//			typeDeclarations.put(pipelines.getName(), pipelines);
//		}
//
//		private void calculateShaderResourceUri(ShaderResource res) {
//			if (res.getClassUrl() != null)
//				return;
//
//			String cname = res.getUri().replace(".", "_");
//			res.setClassUrl(new ClassUrl(cname, "gen/shader/" + cname + ".h"));
//		}
//
//		private void writeShaderResourceHeader(ShaderResource res) {
//			Map<String, Object> vals = new HashMap<>();
//			vals.put("shader_resource_base_classname", res.getClassUrl().getExactClassName());
//			vals.put("shader", res);
//			shaderGenSourcesDir.add(new TemplatedSourceModularFile(res.getClassUrl().getExactClassName() + ".h",
//					new TemplatedSource(descriptor::getInputStream, "gen/shader/template_shader_resource_base.h"))
//							.setValueMap(vals));
//		}
//
//		private void translateShaders() {
//			for (ShaderProgram prog : shaders.getPrograms()) {
//				prog.setClassUrl(new ClassUrl(prog.getName(), "gen/shader/" + prog.getName() + ".h"));
//				prog.addShaderReferences();
//			}
//			for (ShaderResource shadres : shaders.getShaders()) {
//				calculateShaderResourceUri(shadres);
//				writeShaderResourceHeader(shadres);
//			}
//			for (ShaderProgram prog : shaders.getPrograms()) {
//				ShaderResource vertexShader = prog.getVertexShader();
//				ShaderResource fragmentShader = prog.getFragmentShader();
//
//				Map<String, Object> vals = new HashMap<>();
//				vals.put("program", prog);
//				vals.put("vertex", vertexShader);
//				vals.put("fragment", fragmentShader);
//				shaderGenSourcesDir.add(new TemplatedSourceModularFile(prog.getName() + ".h",
//						new TemplatedSource(descriptor::getInputStream, "gen/shader/template_shader_program_base.h")
//								.setHandler(new TranslationHandler() {
//									@Override
//									public void replaceKey(String key, OutputStream os) throws IOException {
//										PrintStream out = new PrintStream(os);
//										switch (key) {
//											case "shader_program_base_inputlayout_members": {
//												List<VariableDeclaration> inputs = vertexShader.getInputVariables();
//												// for (final VariableDeclaration in : inputs) {
//												// String inname = in.getName();
//												// out.println("render::VertexBuffer* buf_" + inname + " = nullptr;");
//												// out.println("int stride_" + inname + " = 0;");
//												// out.println("int offset_" + inname + " = 0;");
//												// }
//												for (int i = 1; i <= inputs.size(); i++) {
//													out.println("template<"
//															+ getStringIndexedWithComma("typename InputType", 0, i)
//															+ (i == 1 ? " = VertexInput>" : ">"));
//													out.println("void setLayout(" + getStringIndexedWithComma(
//															"const Resource<render::VertexBuffer>& buffer", 0, i)
//															+ ") {");
//													// out.println(
//													// "render::VertexBuffer* bufs[]={" + getStringIndexedWithComma("&buffer", 0, i) +
//													// "};");
//													// out.println();
//													if (i < inputs.size()) {
//														out.println("for(unsigned int i = " + i
//																+ "; i < bufferCount; ++i){ buffers[i] = nullptr; }");
//													}
//													out.println("bufferCount = " + i + ";");
//													for (int j = 0; j < i; j++) {
//														out.println("buffers[" + j + "] = buffer" + j + ";");
//														out.println("strides[" + j + "] = sizeof(InputType" + j + ");");
//													}
//													for (int j = 0; j < inputs.size(); j++) {
//														VariableDeclaration in = inputs.get(j);
//														String inname = in.getName();
//														String indexstring = "firstindex_of_type_check<has_public_member_"
//																+ inname + ","
//																+ getStringIndexedWithComma("InputType", 0, i) + ">";
//														String indextype = inname + "_indextype";
//														out.println("typedef " + indexstring + " " + indextype + ";");
//														out.println("typedef typename " + indextype + "::FoundType "
//																+ indextype + "_buftype;");
//														out.println("static_assert(" + indextype
//																+ "::value >= 0, \"Passed structure types don't have member named: "
//																+ inname + "\");");
//														// out.println("buf_" + inname + " = bufs[" + indextype + "::value];");
//														// out.println("stride_" + inname + " = sizeof(" + indextype + "_buftype);");
//														// out.println("offset_" + inname + " = offsetof(" + indextype + "_buftype, " + inname +
//														// ");");
//														out.println("inputs[" + j + "].bufferIndex = " + indextype
//																+ "::value;");
//														out.println("inputs[" + j + "].offset = offsetof(" + indextype
//																+ "_buftype, " + inname + ");");
//														out.println();
//													}
//													// assert buffer necessity
//													for (int j = 0; j < i; j++) {
//														SeparatorBuffer condition = new SeparatorBuffer(" || ");
//														for (VariableDeclaration in : inputs) {
//															condition.add(
//																	j + " == " + in.getName() + "_indextype::value");
//														}
//														out.println("static_assert(" + condition + ", \"buffer" + j
//																+ " with InputType" + j + " is redundant.\");");
//													}
//													out.println("}");
//												}
//												break;
//											}
//											default: {
//												TranslationHandler.super.replaceKey(key, out);
//												break;
//											}
//										}
//									}
//								})).setValueMap(vals));
//			}
//		}
//	}
//
//	private static String getStringIndexedWithComma(String s, int beginindex, int endindex) {
//		return getStringIndexedWithDivider(s, beginindex, endindex, ",");
//	}
//
//	private static String getStringIndexedWithDivider(String s, int beginindex, int endindex, String divider) {
//		String result = "";
//		for (int i = beginindex; i < endindex; i++) {
//			result += s;
//			result += i;
//			if (i + 1 < endindex) {
//				result += divider;
//			}
//		}
//		return result;
//	}
//}
