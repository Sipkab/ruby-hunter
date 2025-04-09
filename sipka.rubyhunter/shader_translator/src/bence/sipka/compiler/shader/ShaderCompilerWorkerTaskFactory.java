package bence.sipka.compiler.shader;

import java.io.Externalizable;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;
import java.util.function.Supplier;
import java.util.stream.Collectors;

import bence.sipka.compiler.shader.elements.VariableDeclaration;
import bence.sipka.compiler.shader.translator.DirectX11ShaderTranslator;
import bence.sipka.compiler.shader.translator.OpenGl30ShaderTranslator;
import bence.sipka.compiler.shader.translator.OpenglEs20ShaderTranslator;
import bence.sipka.compiler.shader.translator.ShaderTranslator;
import bence.sipka.compiler.shader.translator2.DirectX11ShaderTranslator2;
import bence.sipka.compiler.shader.translator2.OpenGlEsShaderTranslator2;
import bence.sipka.compiler.shader.translator2.OpenGlShaderTranslator2;
import bence.sipka.compiler.shader.translator2.ShaderTranslator2;
import bence.sipka.compiler.shader.translator2.program.ShaderEnvironment;
import bence.sipka.compiler.source.SourceTemplateTranslator.TranslationHandler;
import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.TemplatedSourceSakerFile;
import bence.sipka.compiler.types.enums.EnumType;
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
import saker.build.thirdparty.saker.util.StringUtils;
import saker.build.thirdparty.saker.util.io.SerialUtils;
import saker.build.thirdparty.saker.util.thread.ThreadUtils;
import saker.build.trace.BuildTrace;
import sipka.syntax.parser.model.ParseFailedException;
import sipka.syntax.parser.model.rule.Language;
import sipka.syntax.parser.model.statement.Statement;

public class ShaderCompilerWorkerTaskFactory implements TaskFactory<ShaderCompilerTaskFactory.Output>,
		Task<ShaderCompilerTaskFactory.Output>, Externalizable {
	private static final long serialVersionUID = 1L;

	public static final BundleResourceSupplier descriptor = BundleContentAccess
			.getBundleResourceSupplier("shader_translator");
	private static Language UNIFIED_SHADER_LANG;

	private static final Map<String, Supplier<ShaderTranslator>> TRANSLATOR_MAP = new TreeMap<>(
			StringUtils::compareStringsNullFirstIgnoreCase);
	private static final Map<String, Supplier<ShaderTranslator2>> TRANSLATOR2_MAP = new TreeMap<>(
			StringUtils::compareStringsNullFirstIgnoreCase);
	static {
		TRANSLATOR_MAP.put("opengles20", OpenglEs20ShaderTranslator::new);
		TRANSLATOR_MAP.put("opengl30", OpenGl30ShaderTranslator::new);
		TRANSLATOR_MAP.put("directx11", DirectX11ShaderTranslator::new);

		TRANSLATOR2_MAP.put("opengles20", OpenGlEsShaderTranslator2::new);
		TRANSLATOR2_MAP.put("opengl30", OpenGlShaderTranslator2::new);
		TRANSLATOR2_MAP.put("directx11", DirectX11ShaderTranslator2::new);
	}

	private synchronized static Language getUnifiedShaderLanguage() throws IOException {
		if (UNIFIED_SHADER_LANG == null) {
			try (InputStream is = descriptor.getInputStream("unisl.lang")) {
				UNIFIED_SHADER_LANG = Language.fromInputStream(is).get("unified_shading_lang");
			} catch (ParseFailedException e) {
				throw new IOException(e);
			}
		}
		return UNIFIED_SHADER_LANG;
	}

	private Set<FileCollectionStrategy> input;
	private Set<String> renderApi;

	/**
	 * For {@link Externalizable}.
	 */
	public ShaderCompilerWorkerTaskFactory() {
	}

	public ShaderCompilerWorkerTaskFactory(Set<FileCollectionStrategy> input, Set<String> renderApiOption) {
		this.input = input;
		this.renderApi = renderApiOption;
	}

	@Override
	public Task<? extends ShaderCompilerTaskFactory.Output> createTask(ExecutionContext executioncontext) {
		return this;
	}

	@SuppressWarnings("deprecation")
	@Override
	public int getRequestedComputationTokenCount() {
		return 1;
	}

	@Override
	public ShaderCompilerTaskFactory.Output run(TaskContext taskcontext) throws Exception {
		if (saker.build.meta.Versions.VERSION_FULL_COMPOUND >= 8_006) {
			BuildTrace.classifyTask(BuildTrace.CLASSIFICATION_WORKER);
			LinkedHashMap<Object, Object> vals = new LinkedHashMap<>();
			vals.put("Render API", renderApi);
			BuildTrace.setValues(vals, BuildTrace.VALUE_CATEGORY_TASK);
		}
		taskcontext.setStandardOutDisplayIdentifier(ShaderCompilerTaskFactory.TASK_NAME);

		ShaderCollection shaders = new ShaderCollection();
		ShaderEnvironment shaderEnv = new ShaderEnvironment();

		SakerDirectory taskbuilddir = SakerPathFiles.requireBuildDirectory(taskcontext);
		SakerDirectory outputdir = taskbuilddir.getDirectoryCreate(ShaderCompilerTaskFactory.TASK_NAME);
		outputdir.clear();
		SakerDirectory cppoutdir = outputdir.getDirectoryCreate("cpp");
		SakerDirectory shaderGenSourcesDir = cppoutdir.getDirectoryCreate("gen").getDirectoryCreate("shader");
		SakerDirectory shadersDir = outputdir.getDirectoryCreate("shaders");

		NavigableMap<SakerPath, SakerFile> inputfiles = taskcontext.getTaskUtilities()
				.collectFilesReportInputFileAndAdditionDependency(null, input);

		Language shaderlanguage = getUnifiedShaderLanguage();
		ThreadUtils.runParallelItems(inputfiles.values(), f -> {
			try {
				Statement parsed;
				try (InputStream is = f.openInputStream()) {
					parsed = shaderlanguage.parseInputStream(is).getStatement();
				}
				for (Statement shadernodes : parsed.scopeTo("shader_node")) {
					synchronized (shaderEnv) {
						shaderEnv.addShader(shadernodes);
					}
					synchronized (shaders) {
						ShaderProgram program = new ShaderProgram(shadernodes, shaders);
						shaders.setDefiningFile(program, f);
						program.setFile(f);
						shaders.addProgram(program);
					}
				}
			} catch (ParseFailedException | IOException e) {
				throw new RuntimeException("Error in file: " + f.getName(), e);
			}
		});

		// TODO itt a translatorok az enum ertekuk szerint legyenek rendezve
		Collection<ShaderTranslator> translators = renderApi.stream().map(api -> TRANSLATOR_MAP.get(api).get())
				.sorted((a, b) -> a.getUniqueName().compareTo(b.getUniqueName())).collect(Collectors.toList());

		Collection<ShaderTranslator2> translators2 = renderApi.stream().map(api -> TRANSLATOR2_MAP.get(api).get())
				.sorted((a, b) -> a.getUniqueName().compareTo(b.getUniqueName())).collect(Collectors.toList());

		translateShaders(shaderGenSourcesDir, shaders);

		Map<String, Object> shadersmap = new LinkedHashMap<>();
		shadersmap.put("rendererconfigs",
				translators.stream().map(r -> r.getUniqueName()).collect(Collectors.toCollection(ArrayList::new)));
		shadersmap.put("programnames",
				shaders.getPrograms().stream().map(p -> p.getName()).collect(Collectors.toCollection(ArrayList::new)));
		shadersmap.put("shaderclassnames", shaders.getShaders().stream().map(s -> s.getClassUrl().getExactClassName())
				.collect(Collectors.toCollection(ArrayList::new)));

		shaderGenSourcesDir.add(new TemplatedSourceSakerFile("shaders.h",
				new TemplatedSource(descriptor::getInputStream, "gen/shader/shaders.h")));
		TemplatedSourceSakerFile shaderscpp = new TemplatedSourceSakerFile("shaders.cpp",
				new TemplatedSource(descriptor::getInputStream, "gen/shader/shaders.cpp")).setValueMap(shadersmap);
		shaderGenSourcesDir.add(shaderscpp);

		NavigableMap<String, SakerPath> applicationAssets = new TreeMap<>();

		for (ShaderTranslator translator : translators) {
			String uniquename = translator.getUniqueName().toLowerCase(Locale.ENGLISH);
			SakerDirectory translatordir = shaderGenSourcesDir.getDirectoryCreate(uniquename);
			SakerDirectory shadersourcesdir = shadersDir.getDirectoryCreate(uniquename);
			translator.translate(taskcontext, translatordir, shadersourcesdir, shaders, applicationAssets);
		}

		EnumType unifiedshaders = new EnumType("UnifiedShaders");
		EnumType pipelines = new EnumType("UnifiedShaderPipelineStage");
		int counter;
		counter = 0;
		for (ShaderProgram prog : shaders.getPrograms()) {
			unifiedshaders.add(prog.getName(), counter++);
		}
		counter = 0;
		for (ShaderResource shader : shaders.getShaders()) {
			pipelines.add(shader.getClassUrl().getExactClassName(), counter++);
		}

		NavigableMap<String, bence.sipka.compiler.types.TypeDeclaration> typeDeclarations = new TreeMap<>();
		typeDeclarations.put(unifiedshaders.getName(), unifiedshaders);
		typeDeclarations.put(pipelines.getName(), pipelines);

		taskcontext.getTaskUtilities().reportOutputFileDependency(null, SakerPathFiles.toFileContentMap(
				outputdir.getFilesRecursiveByPath(outputdir.getSakerPath(), DirectoryVisitPredicate.everything())));

		outputdir.synchronize();

		//TODO outputs as assets

		return new ShaderCompilerTaskFactory.Output(cppoutdir.getSakerPath(), typeDeclarations, applicationAssets);
	}

	private static void writeShaderResourceHeader(ShaderResource res, SakerDirectory shaderGenSourcesDir) {
		Map<String, Object> vals = new HashMap<>();
		vals.put("shader_resource_base_classname", res.getClassUrl().getExactClassName());
		vals.put("shader", res);
		shaderGenSourcesDir.add(new TemplatedSourceSakerFile(res.getClassUrl().getExactClassName() + ".h",
				new TemplatedSource(descriptor::getInputStream, "gen/shader/template_shader_resource_base.h"))
						.setValueMap(vals));
	}

	private static void translateShaders(SakerDirectory shaderGenSourcesDir, ShaderCollection shaders) {
		for (ShaderProgram prog : shaders.getPrograms()) {
			prog.addShaderReferences();
		}
		for (ShaderResource shadres : shaders.getShaders()) {
			writeShaderResourceHeader(shadres, shaderGenSourcesDir);
		}
		for (ShaderProgram prog : shaders.getPrograms()) {
			ShaderResource vertexShader = prog.getVertexShader();
			ShaderResource fragmentShader = prog.getFragmentShader();

			Map<String, Object> vals = new HashMap<>();
			vals.put("program", prog);
			vals.put("vertex", vertexShader);
			vals.put("fragment", fragmentShader);
			shaderGenSourcesDir.add(new TemplatedSourceSakerFile(prog.getName() + ".h",
					new TemplatedSource(descriptor::getInputStream, "gen/shader/template_shader_program_base.h")
							.setHandler(new TranslationHandler() {
								@Override
								public void replaceKey(String key, OutputStream os) throws IOException {
									PrintStream out = new PrintStream(os);
									switch (key) {
										case "shader_program_base_inputlayout_members": {
											List<VariableDeclaration> inputs = vertexShader.getInputVariables();
											// for (final VariableDeclaration in : inputs) {
											// String inname = in.getName();
											// out.println("render::VertexBuffer* buf_" + inname + " = nullptr;");
											// out.println("int stride_" + inname + " = 0;");
											// out.println("int offset_" + inname + " = 0;");
											// }
											for (int i = 1; i <= inputs.size(); i++) {
												out.println("template<"
														+ getStringIndexedWithComma("typename InputType", 0, i)
														+ (i == 1 ? " = VertexInput>" : ">"));
												out.println("void setLayout("
														+ getStringIndexedWithComma(
																"const Resource<render::VertexBuffer>& buffer", 0, i)
														+ ") {");
												// out.println(
												// "render::VertexBuffer* bufs[]={" + getStringIndexedWithComma("&buffer", 0, i) +
												// "};");
												// out.println();
												if (i < inputs.size()) {
													out.println("for(unsigned int i = " + i
															+ "; i < bufferCount; ++i){ buffers[i] = nullptr; }");
												}
												out.println("bufferCount = " + i + ";");
												for (int j = 0; j < i; j++) {
													out.println("buffers[" + j + "] = buffer" + j + ";");
													out.println("strides[" + j + "] = sizeof(InputType" + j + ");");
												}
												for (int j = 0; j < inputs.size(); j++) {
													VariableDeclaration in = inputs.get(j);
													String inname = in.getName();
													String indexstring = "firstindex_of_type_check<has_public_member_"
															+ inname + ","
															+ getStringIndexedWithComma("InputType", 0, i) + ">";
													String indextype = inname + "_indextype";
													out.println("typedef " + indexstring + " " + indextype + ";");
													out.println("typedef typename " + indextype + "::FoundType "
															+ indextype + "_buftype;");
													out.println("static_assert(" + indextype
															+ "::value >= 0, \"Passed structure types don't have member named: "
															+ inname + "\");");
													// out.println("buf_" + inname + " = bufs[" + indextype + "::value];");
													// out.println("stride_" + inname + " = sizeof(" + indextype + "_buftype);");
													// out.println("offset_" + inname + " = offsetof(" + indextype + "_buftype, " + inname +
													// ");");
													out.println("inputs[" + j + "].bufferIndex = " + indextype
															+ "::value;");
													out.println("inputs[" + j + "].offset = offsetof(" + indextype
															+ "_buftype, " + inname + ");");
													out.println();
												}
												// assert buffer necessity
												for (int j = 0; j < i; j++) {
													SeparatorBuffer condition = new SeparatorBuffer(" || ");
													for (VariableDeclaration in : inputs) {
														condition.add(j + " == " + in.getName() + "_indextype::value");
													}
													out.println("static_assert(" + condition + ", \"buffer" + j
															+ " with InputType" + j + " is redundant.\");");
												}
												out.println("}");
											}
											break;
										}
										default: {
											TranslationHandler.super.replaceKey(key, out);
											break;
										}
									}
								}
							})).setValueMap(vals));
		}
	}

	private static String getStringIndexedWithComma(String s, int beginindex, int endindex) {
		return getStringIndexedWithDivider(s, beginindex, endindex, ",");
	}

	private static String getStringIndexedWithDivider(String s, int beginindex, int endindex, String divider) {
		String result = "";
		for (int i = beginindex; i < endindex; i++) {
			result += s;
			result += i;
			if (i + 1 < endindex) {
				result += divider;
			}
		}
		return result;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, input);
		SerialUtils.writeExternalCollection(out, renderApi);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		input = SerialUtils.readExternalImmutableLinkedHashSet(in);
		renderApi = SerialUtils.readExternalImmutableLinkedHashSet(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((input == null) ? 0 : input.hashCode());
		result = prime * result + ((renderApi == null) ? 0 : renderApi.hashCode());
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
		ShaderCompilerWorkerTaskFactory other = (ShaderCompilerWorkerTaskFactory) obj;
		if (input == null) {
			if (other.input != null)
				return false;
		} else if (!input.equals(other.input))
			return false;
		if (renderApi == null) {
			if (other.renderApi != null)
				return false;
		} else if (!renderApi.equals(other.renderApi))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "ShaderCompilerWorkerTaskFactory[" + (input != null ? "input=" + input + ", " : "")
				+ (renderApi != null ? "renderApiOption=" + renderApi : "") + "]";
	}

}
