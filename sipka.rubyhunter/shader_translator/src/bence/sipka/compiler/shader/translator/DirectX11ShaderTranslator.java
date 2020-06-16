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
package bence.sipka.compiler.shader.translator;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;

import bence.sipka.compiler.shader.SeparatorBuffer;
import bence.sipka.compiler.shader.ShaderCollection;
import bence.sipka.compiler.shader.ShaderCompilerTaskFactory;
import bence.sipka.compiler.shader.ShaderProgram;
import bence.sipka.compiler.shader.ShaderResource;
import bence.sipka.compiler.shader.elements.FunctionDeclaration;
import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.elements.TypeDeclaration;
import bence.sipka.compiler.shader.elements.UniformDeclaration;
import bence.sipka.compiler.shader.elements.VariableDeclaration;
import bence.sipka.compiler.shader.statement.AssignmentStatement;
import bence.sipka.compiler.shader.statement.BlockStatement;
import bence.sipka.compiler.shader.statement.ShaderStatement;
import bence.sipka.compiler.shader.statement.VarDeclarationStatement;
import bence.sipka.compiler.shader.statement.control.IfControlStatement;
import bence.sipka.compiler.shader.statement.control.WhileControlStatement;
import bence.sipka.compiler.shader.statement.expression.BoolLiteral;
import bence.sipka.compiler.shader.statement.expression.ExpressionStatement;
import bence.sipka.compiler.shader.statement.expression.FloatLiteral;
import bence.sipka.compiler.shader.statement.expression.FunctionCallStatement;
import bence.sipka.compiler.shader.statement.expression.IntegerLiteral;
import bence.sipka.compiler.shader.statement.expression.ParenthesesStatement;
import bence.sipka.compiler.shader.statement.expression.VariableExpression;
import bence.sipka.compiler.source.SourceModularFile;
import bence.sipka.compiler.source.SourceTemplateTranslator.TranslationHandler;
import bence.sipka.compiler.source.SourceWritable;
import bence.sipka.compiler.source.TemplatedSource;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.content.SerializableContentDescriptor;
import saker.build.file.path.SakerPath;
import saker.build.file.provider.LocalFileProvider;
import saker.build.task.TaskContext;
import saker.build.thirdparty.saker.util.io.ByteSink;
import saker.build.thirdparty.saker.util.io.StreamUtils;
import saker.build.thirdparty.saker.util.thread.ThreadUtils;
import saker.sdk.support.api.SDKReference;
import saker.sdk.support.api.SDKSupportUtils;
import saker.windows.api.SakerWindowsUtils;

public class DirectX11ShaderTranslator extends ShaderTranslator {

	public static final String HLSL_SHADER_TYPE_VERTEX = "vs_4_0_level_9_1";
	public static final String HLSL_SHADER_TYPE_PIXEL = "ps_4_0_level_9_1";

	private ShaderCollection shaders;

	public DirectX11ShaderTranslator() {
		super("DirectX11");
	}

//	private static final class HlslCompileProcessTask extends ProcessTask {
//		private static final long serialVersionUID = 1L;
//		private transient Module module;
//		private transient SakerFile hlsl;
//		private transient Path outputPath;
//
//		private HlslCompileProcessTask(List<String> commands, Module module, SakerFile hlsl, Path outputpath) {
//			super(commands);
//			this.module = module;
//			this.hlsl = hlsl;
//			this.outputPath = outputpath;
//		}
//
//		@Override
//		protected List<String> getCommands(List<String> commands) throws IOException {
//			List<String> result = new ArrayList<>(commands);
//			result.add(module.getContext().unpack(hlsl).toString());
//			return result;
//		}
//
//		@Override
//		protected void setupProcess(ProcessBuilder builder) throws IOException {
//			super.setupProcess(builder);
//			Files.createDirectories(outputPath.getParent());
//		}
//	}

	private class HlslSourceModularFile extends SourceModularFile {
		private ShaderResource shader;

		public HlslSourceModularFile(String name, ShaderResource shader) {
			super(name, new SourceWritable() {
				@Override
				public void write(OutputStream out) {
					translate(shader, new PrintStream(out));
				}
			});
			this.shader = shader;
			setContentDescriptor(new SerializableContentDescriptor(shader));
		}

		public ShaderResource getShader() {
			return shader;
		}
	}

	@Override
	public synchronized void translate(TaskContext taskcontext, SakerDirectory sourcesdir, SakerDirectory shadersdir,
			ShaderCollection shadersparam, NavigableMap<String, SakerPath> assets) throws IOException {
		NavigableMap<String, SakerPath> syncassets = Collections.synchronizedNavigableMap(assets);
		// TODO uniform constant buffer packing, sorting
		this.shaders = shadersparam.deepCopy();
		// this.shaders = shadersparam;
		replaceTypes();
		SakerDirectory shadergendir = sourcesdir;
		shadergendir.clear();

		Collection<ShaderResource> shadercollection = shaders.getShaders();
		if (shadercollection.size() == 0) {
			return;
		}

		for (final ShaderResource shader : shadercollection) {
			analyze(shader);
		}

		Collection<HlslSourceModularFile> hlslfiles = new ArrayList<>();
		for (final ShaderResource shader : shadercollection) {
			if (!shader.isDefined())
				continue;

			if (shader.getReferenceCount() > 0) {
				String classname = shader.getClassUrl().getExactClassName();
				HlslSourceModularFile hlsl = new HlslSourceModularFile(classname + ".hlsl", shader);
				hlslfiles.add(hlsl);
				shadersdir.add(hlsl);
			}
		}
//		shadersdir.add(hlslfiles);

		ThreadUtils.runParallelItems(hlslfiles, hlsl -> {
			String classname = hlsl.getShader().getClassUrl().getExactClassName();

			String type = hlsl.getShader().getType() == ShaderResource.TYPE_FRAGMENT ? HLSL_SHADER_TYPE_PIXEL
					: HLSL_SHADER_TYPE_VERTEX;
			// if (csofile.lastModified() != f.lastModified()) {
			// HlslCompileModule.CompileSingleData data = new HlslCompileModule.CompileSingleData(f.getPath().toFile(), csofile, type);
			//
			// DataCollection datacoll = new DataCollection();
			// datacoll.set("data", data);
			// module.executeModuleOperation(ModuleIdentifier.valueOf(HlslCompileModule.MODULE_URI), HlslCompileModule.OPERATION_COMPILE_SINGLE,
			// datacoll);
			// csofile.setLastModified(f.lastModified());
			// }

//			ProcessResultModularFile csomodularfile = new ProcessResultModularFile(classname + ".cso");
//			shadersdir.add(csomodularfile);

			Path outputpath = taskcontext.getExecutionContext()
					.toMirrorPath(shadersdir.getSakerPath().resolve(classname + ".cso"));

//			Path csofile = csomodularfile.getSakerPath().toPath();
//			csomodularfile.setExpectedProcessResult(0);

			SDKReference wkitssdk = SDKSupportUtils.resolveSDKReference(taskcontext,
					SakerWindowsUtils.getDefaultWindowsKitsSDK());

			SakerPath fxcexec = wkitssdk.getPath(SakerWindowsUtils.SDK_WINDOWSKITS_PATH_FXC_X86);
			List<String> proccmd = Arrays.asList(fxcexec.toString(), "/nologo", "/T", type, "/Fo",
					outputpath.toString(), taskcontext.mirror(hlsl).toString());
			ProcessBuilder pb = new ProcessBuilder(proccmd).redirectErrorStream(true);
			Process proc = pb.start();
			proc.getOutputStream().close();
			StreamUtils.copyStream(proc.getInputStream(), ByteSink.toOutputStream(taskcontext.getStandardOut()));
			int excode = proc.waitFor();
			if (excode != 0) {
				throw new IOException("Failed to compile shader: " + excode);
			}
			taskcontext.invalidate(LocalFileProvider.getPathKeyStatic(outputpath));
			SakerFile outputcsosakerfile = taskcontext.getTaskUtilities().createProviderPathFile(
					outputpath.getFileName().toString(), LocalFileProvider.getInstance().getPathKey(outputpath));
			shadersdir.add(outputcsosakerfile);

//			csomodularfile.setProcess(new HlslCompileProcessTask(proccmd, module, hlsl, csofile));

			SakerPath csoassetpath = taskcontext.getTaskWorkingDirectoryPath()
					.relativize(shaders.getDefiningFile(hlsl.getShader()).getSakerPath().getParent()
							.resolve(outputcsosakerfile.getName()));
			String assetname = csoassetpath.toString();
			syncassets.put(assetname, outputcsosakerfile.getSakerPath());

			// write class header
			SourceModularFile shaderClassFile = getShaderHeaderClassFile(hlsl.getShader(), classname + ".h",
					csoassetpath);
			shadergendir.add(shaderClassFile);
		});
		// for (final ShaderResource shader : shadercollection) {
		// if (!shader.isDefined())
		// continue;
		//
		// if (shader.getReferenceCount() > 0) {
		// System.out.println("DirectX11ShaderTranslator.translate() " + shader.getUri());
		// String classname = shader.getClassUrl().getExactClassName();
		// SourceModularFile hlslfile = new SourceModularFile(classname + ".hlsl", new SourceWritable() {
		// @Override
		// public void write(OutputStream out) {
		// translate(shader, new PrintStream(out));
		// }
		// }, shader.getDefiningProgram().getDefiningFileLastModification());
		// shadergendir.addFile(hlslfile);
		// shadergendir.synchronizeChildren(hlslfile);
		//
		// File csofile = new File(shadergendir.getPath().toFile(), classname + ".cso");
		// if (csofile.lastModified() != hlslfile.lastModified()) {
		// HlslCompileModule.CompileSingleData data = new HlslCompileModule.CompileSingleData(hlslfile.getPath().toFile(), csofile,
		// shader.getType() == ShaderResource.TYPE_FRAGMENT ? HlslCompileModule.HLSL_SHADER_TYPE_PIXEL
		// : HlslCompileModule.HLSL_SHADER_TYPE_VERTEX);
		//
		// DataCollection datacoll = new DataCollection();
		// datacoll.set("data", data);
		// module.executeModuleOperation(HlslCompileModule.MODULE_URI, HlslCompileModule.OPERATION_COMPILE_SINGLE, datacoll);
		// csofile.setLastModified(hlslfile.lastModified());
		// }
		//
		// ProjectFile csoprojfile = new FilesystemProjectFile(csofile).setAttribute("application_asset_path", csofile.getName());
		// shadergendir.addFile(csoprojfile);
		//
		// // write class header
		// SourceModularFile shaderClassFile = getShaderClassFile(shader, classname + ".h");
		// shadergendir.addFile(shaderClassFile);
		// System.out.println("DirectX11ShaderTranslator.translate() " + shader.getUri() + " end");
		// }
		// }

		for (ShaderProgram prog : shaders.getPrograms()) {
			if (prog.isCompleteProgram()) {
				// TODO
				// prog.validate();
				SourceModularFile programClassFile = getProgramClassFile(prog, prog.getName() + ".h");
				shadergendir.add(programClassFile);
			}
		}
	}

	private void analyze(ShaderResource shader) {
		String searchType;
		String replaceName;

		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			searchType = ShaderCollection.getVertexPositionOutputTypeName();
			replaceName = "result.pos";
		} else if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
			searchType = ShaderCollection.getFragmentColorOutputTypeName();
			replaceName = "result";
		} else {
			throw new RuntimeException("Output variable not found on shader: " + shader.getUri());
		}

		boolean had = false;
		for (Iterator<VariableDeclaration> it = shader.getOutputVariables().iterator(); it.hasNext();) {
			VariableDeclaration o = it.next();
			if (o.getType().getDisplayName().equals(searchType)) {
				if (had) {
					throw new RuntimeException("variable with type: " + searchType + " declared more than once");
				}
				o.setName(replaceName);
				// o.setType(shaders.resolveType("float4"));
				it.remove();
				had = true;
			}
		}

		if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
			for (VariableDeclaration input : shader.getInputVariables()) {
				input.setName("input." + input.getName());
			}
		} else {
			for (VariableDeclaration output : shader.getOutputVariables()) {
				output.setName("result." + output.getName());
			}
		}
	}

	private List<VariableDeclaration> reorderUniform(UniformDeclaration u) {
		TypeDeclaration float1 = shaders.resolveType("float");
		TypeDeclaration float2 = shaders.resolveType("float2");
		TypeDeclaration float3 = shaders.resolveType("float3");
		TypeDeclaration float4 = shaders.resolveType("float4");

		TypeDeclaration float4x4 = shaders.resolveType("float4x4");
		TypeDeclaration float4x3 = shaders.resolveType("float4x3");
		TypeDeclaration float4x2 = shaders.resolveType("float4x2");

		TypeDeclaration float3x2 = shaders.resolveType("float3x2");
		TypeDeclaration float3x3 = shaders.resolveType("float3x3");
		TypeDeclaration float3x4 = shaders.resolveType("float3x4");

		TypeDeclaration float2x2 = shaders.resolveType("float2x2");
		TypeDeclaration float2x3 = shaders.resolveType("float2x3");
		TypeDeclaration float2x4 = shaders.resolveType("float2x4");

		TypeDeclaration texture2d = shaders.resolveType("texture2D");
		TypeDeclaration texture3d = shaders.resolveType("texture3D");

		List<TypeDeclaration> matrices = Arrays.asList(float4x4, float4x3, float4x2, float3x2, float3x3, float3x4,
				float2x2, float2x3, float2x4);
		// matrix mindig 0-n kezdodik
		// utana meg lehet rakni a legvegere
		// float1x1 sima float lesz
		// float3x1 sima float3 lesz
		List<VariableDeclaration> members = new ArrayList<>(u.getMembers());

		List<VariableDeclaration> onemems = new ArrayList<>();
		List<VariableDeclaration> twomems = new ArrayList<>();
		List<VariableDeclaration> nonInternals = new ArrayList<>();

		List<VariableDeclaration> aligned = new ArrayList<>();

		// full aligned types
		for (Iterator<VariableDeclaration> it = members.iterator(); it.hasNext();) {
			VariableDeclaration m = it.next();
			TypeDeclaration type = m.getType();
			if (type.equals(float4) || type.equals(float3) || matrices.contains(type)) {
				it.remove();
				aligned.add(m);
			} else if (float1.equals(type)) {
				it.remove();
				onemems.add(m);
			} else if (float2.equals(type)) {
				it.remove();
				twomems.add(m);
			} else if (texture2d.equals(type) || texture3d.equals(type)) {
				it.remove();
				nonInternals.add(m);
			}
		}
		if (members.size() > 0) {
			throw new RuntimeException("Failed to realign HLSL constant buffers, remaining types: " + members);
		}
		// fill two paddings
		for (int i = 0; i < aligned.size() && twomems.size() > 0; i++) {
			VariableDeclaration current = aligned.get(i);
			TypeDeclaration type = current.getType();
			if (type.equals(float2x2) || type.equals(float2x3) || type.equals(float2x4)) {
				VariableDeclaration next = i + 1 < aligned.size() ? aligned.get(i + 1) : null;
				if (next == null || !float1.equals(next.getType()) || !float2.equals(next.getType())) {
					aligned.add(i + 1, twomems.remove(0));
				}
			}
		}

		// filling 1 paddings after float3s or two paddings after float2-s
		for (int i = 0; i < aligned.size() && onemems.size() > 0; i++) {
			VariableDeclaration current = aligned.get(i);
			TypeDeclaration type = current.getType();
			if (type.equals(float3) || type.equals(float3x2) || type.equals(float3x3) || type.equals(float3x4)) {
				VariableDeclaration next = i + 1 < aligned.size() ? aligned.get(i + 1) : null;
				if (next == null || !float1.equals(next.getType())) {
					aligned.add(i + 1, onemems.remove(0));
				}
			} else if (type.equals(float2x2) || type.equals(float2x3) || type.equals(float2x4)) {
				VariableDeclaration next = i + 1 < aligned.size() ? aligned.get(i + 1) : null;
				VariableDeclaration nextafter = i + 2 < aligned.size() ? aligned.get(i + 2) : null;
				if (next == null) {
					aligned.add(i + 1, onemems.remove(0));
					--i;
				} else {
					TypeDeclaration nexttype = next.getType();
					if (!float2.equals(nexttype)
							&& (!float1.equals(nexttype) || nextafter == null || !float1.equals(nextafter.getType()))) {
						aligned.add(i + 1, onemems.remove(0));
						--i;
					}
				}
			}
		}

		aligned.addAll(twomems);
		twomems.clear();

		aligned.addAll(onemems);
		onemems.clear();

		aligned.addAll(nonInternals);

		return aligned;
	}

	private static String shaderTypeToFrameworkType(String shadertype) {
		switch (shadertype) {
			case "float": {
				return "float";
			}
			case "float2": {
				return "Vector2F";
			}
			case "float3": {
				return "Vector3F";
			}
			case "float4": {
				return "Vector4F";
			}
			case "float2x2": {
				return "Matrix<2>";
			}
			case "float3x3": {
				return "Matrix<3>";
			}
			case "float4x4": {
				return "Matrix<4>";
			}
			case "texture2D": {
				return "Resource<render::Texture>";
			}
			default: {
				throw new RuntimeException(
						"Found no type conversion from shader type to framework type: " + shadertype);
			}
		}
	}

	private static String shaderTypeToFrameworkUniformType(String shadertype) {
		switch (shadertype) {
			case "float": {
				return "float";
			}
			case "float2": {
				return "Vector2F";
			}
			case "float3": {
				return "Vector3F";
			}
			case "float4": {
				return "Vector4F";
			}
			case "float2x2": {
				return "HLSLMatrix<2, 2>";
			}
			case "float3x3": {
				return "HLSLMatrix<3, 3>";
			}
			case "float4x4": {
				return "HLSLMatrix<4, 4>";
			}
			case "texture2D": {
				return "Resource<render::Texture>";
			}
			default: {
				throw new RuntimeException(
						"Found no type conversion from shader type to framework type: " + shadertype);
			}
		}
	}

	private static String shaderTypeToRenderType(String gltype) {
		switch (gltype) {
			case "texture2D": {
				return "DirectX11Texture*";
			}
			default: {
				return shaderTypeToFrameworkType(gltype);
			}
		}
	}

	private static int getFloatSizeInUniformOfFrameworkType(TypeDeclaration type) {
		switch (type.getDeclaredName()) {
			case "float": {
				return 1;
			}
			case "float2": {
				return 2;
			}
			case "float3": {
				return 3;
			}
			case "float4": {
				return 4;
			}
			case "float2x2": {
				return 2 * 2;
			}
			case "float3x3": {
				return 3 * 3;
			}
			case "float4x4": {
				return 4 * 4;
			}
			default: {
				// throw new RuntimeException("Unknown float count of fw type: " + type);
				return 0;
			}
		}
	}

	private static boolean zeroAlignedHlsl(TypeDeclaration type) {
		return isMatrixType(type);
	}

	private static int getRemainingHlslOffsetAfterType(TypeDeclaration type) {
		switch (type.getDeclaredName()) {
			case "float":
				return 3;
			case "float2":
				return 2;
			case "float3":
				return 1;
			case "float4":
				return 0;
			case "float2x2":
			case "float2x3":
			case "float2x4":
				return 2;
			case "float3x2":
			case "float3x3":
			case "float3x4":
				return 1;
			case "float4x2":
			case "float4x3":
			case "float4x4":
				return 0;
			default: {
				throw new RuntimeException("Cannot determine hlsl offset for type: " + type);
			}
		}
	}

	private static boolean isMatrixType(TypeDeclaration type) {
		switch (type.getDeclaredName()) {
			case "float2x2":
			case "float2x3":
			case "float2x4":
			case "float3x2":
			case "float3x3":
			case "float3x4":
			case "float4x2":
			case "float4x3":
			case "float4x4":
				return true;
			default:
				return false;
		}
	}

	private static String shaderTypeToDxgiFormat(String shadertype) {
		switch (shadertype) {
			case "float": {
				return "DXGI_FORMAT_R32_FLOAT";
			}
			case "float2": {
				return "DXGI_FORMAT_R32G32_FLOAT";
			}
			case "float3": {
				return "DXGI_FORMAT_R32G32B32_FLOAT";
			}
			case "float4": {
				return "DXGI_FORMAT_R32G32B32A32_FLOAT";
			}
			case "float3x3": {
				throw new RuntimeException("matrices not yet supported");
			}
			case "float4x4": {
				throw new RuntimeException("matrices not yet supported");
			}
			case "texture2D": {
				throw new RuntimeException("texture dxgi format not found");
			}
			default: {
				throw new RuntimeException(
						"Found no type conversion from shader type to framework type: " + shadertype);
			}
		}
	}

	private static boolean passFrameworkTypeByValue(String frameworktype) {
		return frameworktype.endsWith("*");
	}

	private static String getIndexedWithComma(String s, int beginindex, int endindex) {
		String result = "";
		for (int i = beginindex; i < endindex; i++) {
			result += s;
			result += i;
			if (i + 1 < endindex) {
				result += ",";
			}
		}
		return result;
	}

	private static String getShaderPipelineSetPrefix(ShaderResource res) {
		switch (res.getType()) {
			case ShaderResource.TYPE_FRAGMENT: {
				return "PS";
			}
			case ShaderResource.TYPE_VERTEX: {
				return "VS";
			}
			default: {
				throw new RuntimeException("Invalid type: " + res.getType());
			}
		}
	}

	private SourceModularFile getShaderHeaderClassFile(ShaderResource shader, String name, SakerPath csofilepath) {
		String classname = shader.getClassUrl().getExactClassName();

		Map<String, Object> valmap = new HashMap<>();
		valmap.put("shader_resource_h_guard", (getUniqueName() + classname).toUpperCase() + "_H_");
		valmap.put("shader_resource_h_classname", classname);
		valmap.put("shader_resource_h_assetpath", csofilepath.toString().replace('.', '_').replaceAll("[/\\\\]", "::"));
		valmap.put("shader_resource_h_shaderfilename", classname + ".cso");
		valmap.put("shader_resource_h_comptr_type",
				shader.getType() == ShaderResource.TYPE_FRAGMENT ? "PixelShader" : "VertexShader");

		TemplatedSource source = new TemplatedSource(ShaderCompilerTaskFactory.descriptor::getInputStream,
				"gen/shader/" + getUniqueName().toLowerCase() + "/template_shader_resource.h").setValueMap(valmap)
						.setHandler(new TranslationHandler() {
							@Override
							public void replaceKey(String key, OutputStream os) throws IOException {
								PrintStream out = new PrintStream(os);
								switch (key) {
									case "shader_resource_h_public_members": {
										int cbuffercounter = 0;
										int samplercounter = 0;
										int texturecounter = 0;
										TypeDeclaration texture2d = shaders.resolveType("texture2D");
										TypeDeclaration texture3d = shaders.resolveType("texture3D");
										for (UniformDeclaration u : shader.getUniforms()) {
											String assertions = "ASSERT(renderer->devcon != nullptr);\n";
											List<TypeDeclaration> notinternaltypes = Arrays.asList(texture2d,
													texture3d);
											List<VariableDeclaration> reordered = reorderUniform(u);
											List<VariableDeclaration> textures = new ArrayList<>();
											List<VariableDeclaration> internals = new ArrayList<>();
											SeparatorBuffer texturesseparator = new SeparatorBuffer(", ");
											SeparatorBuffer samplersseparator = new SeparatorBuffer(", ");
											String notInternalMembers = "";
											String internalMembers = "";
											int internalRemainingOffset = 0;
											int paddingid = 0;
											for (VariableDeclaration m : reordered) {
												TypeDeclaration type = m.getType();
												String name = m.getName();
												if (notinternaltypes.contains(type)) {
													if (type.equals(texture2d)) {
														++samplercounter;
														++texturecounter;
														notInternalMembers += shaderTypeToRenderType(
																type.getDeclaredName()) + " " + name + " = nullptr;"
																+ System.lineSeparator();
														texturesseparator.add("static_cast<DirectX11Texture*>(" + name
																+ ")->shaderResource");
														samplersseparator.add("static_cast<DirectX11Texture*>(" + name
																+ ")->samplerState");
														assertions += "ASSERT(" + name + " != nullptr);\n";
														assertions += "ASSERT(static_cast<DirectX11Texture*>(" + name
																+ ")->shaderResource != nullptr);\n";
														assertions += "ASSERT(static_cast<DirectX11Texture*>(" + name
																+ ")->samplerState != nullptr);\n";
														textures.add(m);
													} else {
														throw new RuntimeException("Uninplemented type: " + type);
													}
												} else {
													if (isMatrixType(type)) {
														if (internalRemainingOffset > 0) {
															internalMembers += "float padding" + paddingid++ + "["
																	+ internalRemainingOffset + "];"
																	+ System.lineSeparator();
														}
														internalRemainingOffset = getRemainingHlslOffsetAfterType(type);
													} else {
														// vector type
														if (internalRemainingOffset > 0) {
															if (internalRemainingOffset < getFloatSizeInUniformOfFrameworkType(
																	type)) {
																internalMembers += "float padding" + paddingid++ + "["
																		+ internalRemainingOffset + "];"
																		+ System.lineSeparator();
																internalRemainingOffset = getRemainingHlslOffsetAfterType(
																		type);
															} else {
																internalRemainingOffset -= getFloatSizeInUniformOfFrameworkType(
																		type);
															}
														} else {
															internalRemainingOffset = getRemainingHlslOffsetAfterType(
																	type);
														}
													}
													internalMembers += shaderTypeToFrameworkUniformType(
															type.getDeclaredName()) + " " + name + ";"
															+ System.lineSeparator();
													internals.add(m);
												}
											}
											if (internalRemainingOffset > 0) {
												internalMembers += "float padding" + paddingid++ + "["
														+ internalRemainingOffset + "];" + System.lineSeparator();
											}

											String uname = u.getName();
											out.println(
													"class " + uname + "_impl final: public " + uname + " { public:");
											if (internalMembers.length() > 0) {
												out.println("class InternalRepresentation{ public: ");
												out.println(internalMembers);
												out.println(
														"InternalRepresentation(const " + uname + "_value& value) {");
												for (VariableDeclaration intmember : internals) {
													out.println("this->" + intmember.getName() + " = value."
															+ intmember.getName() + ";");
												}
												out.println("}");
												out.println("};");
												out.println(
														"static_assert(sizeof(InternalRepresentation) % 16 == 0, \"Size of uniform (constant buffer) is not multiple of 16 bytes\");");
												// out.println("internal;");
												out.println("DirectX11ConstantBuffer cbuffer;");
											}
											// out.println(notInternalMembers);
											out.println("DirectX11Renderer* renderer;");

											// TODO make constant buffer loadable
											out.println("virtual bool load() override { return "
													+ (internalMembers.length() > 0 ? "cbuffer.load()" : "true")
													+ "; }");
											out.println("virtual void free() override { "
													+ (internalMembers.length() > 0 ? "cbuffer.free();" : "") + " }");
											out.println("virtual bool reload() override { free(); return load(); }");
											out.println("virtual void update() override { ");
											if (internalMembers.length() > 0) {
												out.println("InternalRepresentation internal { *this };");
												out.println("cbuffer.update(&internal);");
											}
											if (notInternalMembers.length() > 0) {
												for (VariableDeclaration tex : textures) {
													// out.println(tex.getName() + " = static_cast<DirectX11Texture*>(this->" + tex.getName() + ");");
												}
											}
											out.println("}");
											out.println("void set() {");
											out.println(assertions);
											String pplprefix = getShaderPipelineSetPrefix(u.getParentShader());
											if (internalMembers.length() > 0) {
												final int cbufferslot = cbuffercounter++;
												out.println(
														"ID3D11Buffer* buf = cbuffer.getDirectX11Name(); ASSERT(buf != nullptr);");
												out.println("renderer->devcon->" + pplprefix + "SetConstantBuffers("
														+ cbufferslot + ", 1, &buf);");
											}
											if (notInternalMembers.length() > 0) {
												out.println("ID3D11ShaderResourceView* srviews[" + texturecounter
														+ "] {" + texturesseparator + "};");
												out.println("ID3D11SamplerState* samplers[" + samplercounter + "] {"
														+ samplersseparator + "};");
												out.println("renderer->devcon->" + pplprefix + "SetShaderResources(0, "
														+ texturecounter + ", srviews);");
												out.println("renderer->devcon->" + pplprefix + "SetSamplers(0, "
														+ samplercounter + ", samplers);");
											}
											out.println("}");

											out.println(uname
													+ "_impl(DirectX11Renderer* renderer) : renderer { renderer }");
											if (internalMembers.length() > 0) {
												out.println(", cbuffer{renderer, sizeof(InternalRepresentation) }");
											}
											out.println("{ } ");

											out.println("};");
											out.println("virtual " + uname + "_impl* createUniform_" + uname
													+ "Impl() override { return new " + uname + "_impl{ renderer };}");
										}
										break;
									}
									default: {
										break;
									}
								}
							}
						});
		SourceModularFile result = new SourceModularFile(name, source);
		result.setContentDescriptor(new SerializableContentDescriptor(shader));
		return result;
	}

	private SourceModularFile getProgramClassFile(ShaderProgram prog, String name) {
		final ShaderResource vertexShader = prog.getVertexShader();
		final ShaderResource fragmentShader = prog.getFragmentShader();

		String classname = prog.getName();

		Map<String, Object> valmap = new HashMap<>();
		valmap.put("shader_program_h_guard", (getUniqueName() + classname).toUpperCase() + "_H_");
		valmap.put("shader_program_h_classname", classname);
		valmap.put("shader_program_h_vertexshadertype", vertexShader.getClassUrl().getExactClassName());
		valmap.put("shader_program_h_fragmentshadertype", fragmentShader.getClassUrl().getExactClassName());
		valmap.put("shader_program_base_inputlayout_member_count", vertexShader.getInputVariables().size());

		valmap.put("program", prog);
		valmap.put("vertex", vertexShader);
		valmap.put("fragment", fragmentShader);
		valmap.put("translator", this);

		TypeDeclaration texture2d = shaders.resolveType("texture2D");
		TypeDeclaration texture3d = shaders.resolveType("texture3D");

		TemplatedSource source = new TemplatedSource(ShaderCompilerTaskFactory.descriptor::getInputStream,
				"gen/shader/" + getUniqueName().toLowerCase() + "/template_shader_program.h").setValueMap(valmap)
						.setHandler(new TranslationHandler() {
							@Override
							public void replaceKey(String key, OutputStream os) throws IOException {
								PrintStream out = new PrintStream(os);
								switch (key) {
									case "shader_program_h_public_members": {
										// List<TypeDeclaration> notinternal = Arrays.asList(texture2d, texture3d);
										// int vscbuffercounter = 0;
										// int pscbuffercounter = 0;
										// int vssamplercounter = 0;
										// int pssamplercounter = 0;
										// int vstexturecounter = 0;
										// int pstexturecounter = 0;
										// for (UniformDeclaration u : prog.getUniforms()) {
										// List<VariableDeclaration> reordered = reorderUniform(u);
										// String notInternalMembers = "";
										// String internalMembers = "";
										// SeparatorBuffer internalConstructorArgs = new SeparatorBuffer(", ");
										// SeparatorBuffer internalConstructorMemberInitList = new SeparatorBuffer(", ");
										// String uniformname = u.getName();
										// out.printlnPostInc("class " + uniformname + " {");
										// out.println("public:");
										// int internalRemainingOffset = 0;
										// int paddingid = 0;
										// for (VariableDeclaration m : reordered) {
										// TypeDeclaration type = m.getType();
										// String name = m.getName();
										// if (notinternal.contains(type)) {
										// notInternalMembers += shaderTypeToRenderType(type.getDeclaredName()) + " " + name + ";" + System.lineSeparator();
										// } else {
										// if (isMatrixType(type)) {
										// if (internalRemainingOffset > 0) {
										// internalMembers += "float padding" + paddingid++ + "[" + internalRemainingOffset + "];"
										// + System.lineSeparator();
										// }
										// internalRemainingOffset = getRemainingHlslOffsetAfterType(type);
										// } else {
										// // vector type
										// if (internalRemainingOffset > 0) {
										// if (internalRemainingOffset < getFloatSizeInUniformOfFrameworkType(type)) {
										// internalMembers += "float padding" + paddingid++ + "[" + internalRemainingOffset + "];"
										// + System.lineSeparator();
										// internalRemainingOffset = getRemainingHlslOffsetAfterType(type);
										// } else {
										// internalRemainingOffset -= getFloatSizeInUniformOfFrameworkType(type);
										// }
										// } else {
										// internalRemainingOffset = getRemainingHlslOffsetAfterType(type);
										// }
										// }
										// internalMembers += shaderTypeToFrameworkUniformType(type.getDeclaredName()) + " " + name + ";" +
										// System.lineSeparator();
										// internalConstructorArgs.add(shaderTypeToFrameworkType(type.getDeclaredName()) + " const &" + name);
										// internalConstructorMemberInitList.add(name + " { " + name + " }");
										// }
										// }
										// if (internalRemainingOffset > 0) {
										// internalMembers += "float padding" + paddingid++ + "[" + internalRemainingOffset + "];" + System.lineSeparator();
										// }
										// out.println("private: ");
										// out.println("friend class " + prog.getName() + ";");
										// if (internalMembers.length() > 0) {
										// out.printlnPostInc("class InternalRepresentation{");
										// out.println("public:");
										// out.println(internalMembers);
										// out.println("InternalRepresentation() { }");
										// out.println("InternalRepresentation(" + internalConstructorArgs + ") ");
										// out.println(": " + internalConstructorMemberInitList + " { }");
										// out.printlnPreDec("} internaldata;");
										// out.println(
										// "static_assert(sizeof(InternalRepresentation) % 16 == 0, \"Size of uniform (constant buffer) is not multiple of 16
										// bytes\");");
										//
										// }
										// out.println(notInternalMembers);
										// out.println("public: ");
										// out.println(uniformname + "()");
										// if (internalMembers.length() > 0) {
										// // out.println(": buffer { sizeof(InternalRepresentation) }");
										// }
										// out.println("{ }");
										//
										// if (u.getMembers().size() > 0) {
										// SeparatorBuffer constructorargs = new SeparatorBuffer(", ");
										// SeparatorBuffer memberinitlist = new SeparatorBuffer(", ");
										//
										// for (VariableDeclaration m : u.getMembers()) {
										// String fwtype = shaderTypeToFrameworkType(m.getType().getDeclaredName());
										// constructorargs.add(fwtype + " " + (passFrameworkTypeByValue(fwtype) ? "" : "const &") + m.getName());
										// }
										//
										// if (internalMembers.length() > 0) {
										// SeparatorBuffer internalbuf = new SeparatorBuffer(", ");
										// for (VariableDeclaration m : reordered) {
										// if (!notinternal.contains(m.getType())) {
										// internalbuf.add(m.getName());
										// }
										// }
										// if (internalbuf.length() > 0) {
										// memberinitlist.add("internaldata { " + internalbuf + " }");
										// // memberinitlist.add("buffer { sizeof(InternalRepresentation), &internaldata }");
										// }
										// }
										// for (VariableDeclaration m : reordered) {
										// if (notinternal.contains(m.getType())) {
										// memberinitlist.add(m.getName() + " { static_cast<" + shaderTypeToRenderType(m.getType().getDeclaredName()) + ">("
										// + m.getName() + ") }");
										// }
										// }
										// out.println(uniformname + "(" + constructorargs + ")");
										// out.println(": " + memberinitlist.toString());
										// out.println("{ }");
										//
										// out.printlnPostInc("void set(" + constructorargs + ") {");
										// if (internalMembers.length() > 0) {
										// out.println("this->internaldata = { "
										// + new SeparatorBuffer(", ", u.getMembers(), obj -> notinternal.contains(obj.getType()) ? null : obj.getName())
										// + " };");
										//
										// }
										// for (VariableDeclaration v : u.getMembers()) {
										// if (notinternal.contains(v.getType())) {
										// out.println("this->" + v.getName() + " = static_cast<" + shaderTypeToRenderType(v.getType().getDeclaredName())
										// + ">(" + v.getName() + ");");
										// }
										// }
										// out.printlnPreDec("}");
										// }
										// out.printlnPreDec("};");
										// if (internalMembers.length() > 0) {
										// out.println("DirectX11ConstantBuffer cbbuffer" + uniformname + "{ renderer, sizeof(" + uniformname
										// + "::InternalRepresentation) };");
										// }
										//
										// out.printlnPostInc("void setUniform(" + uniformname + "& u) {");
										// {
										// // out.println("ASSERT(isInUse()) << \"Program is not in use\";");
										// final String setConstantBuffersPrefix;
										// final int shadertype = u.getParentShader().getType();
										// if (shadertype == ShaderResource.TYPE_VERTEX) {
										// setConstantBuffersPrefix = "VS";
										// } else {
										// // fragment shader
										// setConstantBuffersPrefix = "PS";
										// }
										// if (internalMembers.length() > 0) {
										// final int cbufferslot;
										// if (shadertype == ShaderResource.TYPE_VERTEX) {
										// cbufferslot = vscbuffercounter++;
										// } else {
										// // fragment shader
										// cbufferslot = pscbuffercounter++;
										// }
										// out.println("ID3D11Buffer* buffer = cbbuffer" + uniformname + ".getDirectX11Name();");
										// out.println("cbbuffer" + uniformname + ".update(&u.internaldata);");
										// out.println("renderer->devcon->" + setConstantBuffersPrefix + "SetConstantBuffers(" + cbufferslot + ", 1,
										// &buffer);");
										// }
										// int codesampler = 0;
										// for (VariableDeclaration m : reordered) {
										// TypeDeclaration type = m.getType();
										// String name = m.getName();
										// final int srcount;
										// final int sampcount;
										// if (shadertype == ShaderResource.TYPE_VERTEX) {
										// srcount = vstexturecounter++;
										// sampcount = vssamplercounter;
										// } else {
										// srcount = pstexturecounter++;
										// sampcount = pssamplercounter;
										// }
										// if (type.equals(texture2d)) {
										// out.println("ID3D11ShaderResourceView* srview" + codesampler + " = u." + name + "->shaderResource;");
										// out.println("ID3D11SamplerState* sampler" + codesampler + " = u." + name + "->samplerState;");
										// out.println("renderer->devcon->" + setConstantBuffersPrefix + "SetShaderResources(" + srcount + ", 1, &srview"
										// + codesampler + ");");
										// out.println("renderer->devcon->" + setConstantBuffersPrefix + "SetSamplers(" + sampcount + ", 1, &sampler"
										// + codesampler + ");");
										// ++codesampler;
										// }
										// }
										// }
										// out.printlnPreDec("}");
										//
										// }

										// for (UniformDeclaration u : prog.getUniforms()) {
										// out.printlnPostInc("virtual void set(" + u.getName() + "& u) override {");
										// out.println("static_cast<" + getUniqueName() + u.getParentShader().getClassUrl().getExactClassName() + "::" +
										// u.getName()
										// + "_impl&>(u).set();");
										// out.printlnPreDec("}");
										// }

										break;
									}
									case "shader_program_h_inputlayout_load": {
										List<VariableDeclaration> inputs = vertexShader.getInputVariables();
										for (int i = 0; i < inputs.size(); i++) {
											VariableDeclaration in = inputs.get(i);
											out.println("IED[" + i + "].SemanticName = \"VS" + i + "INPUT\";");
											out.println("IED[" + i + "].SemanticIndex = 0;");
											out.println("IED[" + i + "].Format = "
													+ shaderTypeToDxgiFormat(in.getType().getDeclaredName()) + ";");
											out.println("IED[" + i + "].InputSlot = inputs[" + i + "].bufferIndex;");
											out.println("IED[" + i + "].AlignedByteOffset = inputs[" + i + "].offset;");
											out.println("IED[" + i + "].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;");
											out.println("IED[" + i + "].InstanceDataStepRate = 0;");
										}
										break;
									}
									case "shader_program_h_inputlayout_members": {
										List<VariableDeclaration> inputs = vertexShader.getInputVariables();
										for (int i = 1; i <= inputs.size(); i++) {
											out.println("template<" + getIndexedWithComma("typename InputType", 0, i)
													+ (i == 1 ? " = VertexInput>" : ">"));
											out.println("void setLayout("
													+ getIndexedWithComma("render::VertexBuffer& buffer", 0, i)
													+ ") {");
											out.println("DirectX11VertexBuffer* bufs[]={"
													+ getIndexedWithComma("(DirectX11VertexBuffer*)&buffer", 0, i)
													+ "};");
											out.println();
											out.println("D3D11_INPUT_ELEMENT_DESC IED[" + inputs.size() + "];");
											out.println("this->bufferCount = " + i + ";");
											for (int index = 0; index < inputs.size(); index++) {
												VariableDeclaration in = inputs.get(index);
												String inname = in.getName();
												String indexstring = "firstindex_of_type_check<has_public_member_"
														+ inname + "," + getIndexedWithComma("InputType", 0, i) + ">";
												String indextype = inname + "_indextype";
												out.println("typedef " + indexstring + " " + indextype + ";");
												out.println("typedef typename " + indextype + "::FoundType " + indextype
														+ "_buftype;");
												out.println("static_assert(" + indextype
														+ "::value >= 0, \"Passed structure types don't have member named: "
														+ inname + "\");");

												out.println(
														"IED[" + index + "].SemanticName = \"VS" + index + "INPUT\";");
												// TODO semantic index for matrices
												out.println("IED[" + index + "].SemanticIndex = 0;");
												out.println("IED[" + index + "].Format = "
														+ shaderTypeToDxgiFormat(in.getType().getDeclaredName()) + ";");
												// TODO buffer inputslot
												out.println("IED[" + index + "].InputSlot = " + indextype + "::value;");
												out.println("IED[" + index + "].AlignedByteOffset = offsetof("
														+ indextype + "_buftype, " + inname + ");");
												out.println("IED[" + index
														+ "].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;");
												out.println("IED[" + index + "].InstanceDataStepRate = 0;");
												out.println("this->buffers[" + index + "] = bufs[" + indextype
														+ "::value];");
												out.println("this->strides[" + index + "] = sizeof(" + indextype
														+ "_buftype);");
												out.println();
											}
											out.println("if(layout != nullptr) layout->Release();");
											out.println("ThrowIfFailed(");
											out.println("program->renderer->dev->CreateInputLayout(IED, "
													+ inputs.size()
													+ ", program->vertexShader->getShaderData(), program->vertexShader->getShaderDataSize(), &layout)");
											out.println(");");

											// assert buffer necessity
											for (int j = 0; j < i; j++) {
												SeparatorBuffer sepbuf = new SeparatorBuffer(" || ");
												for (VariableDeclaration in : inputs) {
													sepbuf.add(j + " == " + in.getName() + "_indextype::value");
												}
												out.println("static_assert(" + sepbuf + ", \"buffer" + j
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
						});

		SourceModularFile result = new SourceModularFile(name, source);
		result.setContentDescriptor(new SerializableContentDescriptor(prog));
		return result;
	}

	private void translate(ShaderResource shader, PrintStream os) {
		List<UniformDeclaration> uniforms = new ArrayList<>(shader.getUniforms());
		List<VariableDeclaration> inputs = new ArrayList<>(shader.getInputVariables());
		List<VariableDeclaration> outputs = new ArrayList<>(shader.getOutputVariables());

		int cbuffercounter = 0;
		List<VariableDeclaration> textureUniforms = new ArrayList<>();
		for (UniformDeclaration uni : uniforms) {
			boolean hadMember = false;
			for (VariableDeclaration member : reorderUniform(uni)) {
				if (member.getType().getDeclaredName().equals("texture2D")
						|| member.getType().getDeclaredName().equals("texture3D")) {
					textureUniforms.add(member);
					continue;
				}
				if (!hadMember) {
					os.println("cbuffer " + uni.getName() + " : register(b" + cbuffercounter + ") {");
					++cbuffercounter;
					hadMember = true;
				}
				os.println(member.getType().getDeclaredName() + " " + member.getName() + ";");
			}
			if (hadMember) {
				os.println("};");
			}
		}

		int texturecount = 0;
		for (VariableDeclaration tex : textureUniforms) {
			os.println("Texture2D " + tex.getName() + " : register(t" + texturecount + ");");
			os.println("SamplerState " + tex.getName() + "_sampler : register(s" + texturecount + ");");
			++texturecount;
		}

		os.println("class PSInput {");
		os.println("float4 pos : SV_POSITION;");
		int inputcount = 0;
		for (VariableDeclaration input : shader.getType() == ShaderResource.TYPE_VERTEX ? outputs : inputs) {
			os.println(input.getType().getDeclaredName() + " "
					+ input.getName().substring(input.getName().indexOf('.') + 1) + " : PSINPUT" + inputcount++ + ";");
		}
		os.println("};");

		final String resulttype;
		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			int vertexinputcount = 0;
			resulttype = "PSInput";
			os.print(resulttype + " main(");
			for (Iterator<VariableDeclaration> it = inputs.iterator(); it.hasNext();) {
				VariableDeclaration input = it.next();
				os.print(input.getType().getDeclaredName() + " " + input.getName() + " : VS" + vertexinputcount++
						+ "INPUT");
				if (it.hasNext()) {
					os.print(", ");
				}
			}
			os.println(") {");
		} else {
			resulttype = "float4";
			os.println(resulttype + " main(PSInput input) : SV_TARGET {");
		}
		os.println(resulttype + " result;");
		putBlockStatement(shader.getStatements(), os);

		TypeDeclaration depthtype = shaders.resolveType("depth");
		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			// has depth variable output?
			for (VariableDeclaration o : shader.getOutputVariables()) {
				if (o.getType().equals(depthtype)) {
					os.println(o.getName() + "=" + "result.pos.z/result.pos.w;");
					break;
				}
			}
		}

		os.println("return result;");
		os.println("}");
	}

	private void replaceTypes() {
		shaders.resolveOperator("*", "float4", "float4x4").setName("mul");
		shaders.resolveOperator("*", "float3", "float3x3").setName("mul");
		shaders.resolveOperator("*", "float2", "float2x2").setName("mul");

		shaders.resolveOperator("*", "float4x4", "float4").setName("mul");
		shaders.resolveOperator("*", "float3x3", "float3").setName("mul");
		shaders.resolveOperator("*", "float2x2", "float2").setName("mul");
	}

	@Override
	public void putVariableDeclaration(VarDeclarationStatement vardecl, PrintStream os) {
		ExpressionStatement initialization = vardecl.getInitialization();
		VariableDeclaration declaration = vardecl.getDeclaration();
		os.print(declaration.getType().getDeclaredName() + " " + declaration.getName());
		if (initialization != null) {
			os.print("=");
			initialization.write(this, os);
		}
		os.println(";");
	}

	@Override
	public void putFloatLiteral(FloatLiteral floatLiteral, PrintStream os) {
		os.print(floatLiteral.getValue());
	}

	@Override
	public void putIntegerLiteral(IntegerLiteral integerLiteral, PrintStream os) {
		os.print(integerLiteral.getValue());
	}

	@Override
	public void putBoolLiteral(BoolLiteral boolLiteral, PrintStream os) {
		os.print(boolLiteral.getBoolValue() ? "true" : "false");
	}

	@Override
	public void putVariable(VariableExpression variable, PrintStream os) {
		os.print(variable.getVariable().getName());
		for (ScopeDeclaration scopeDeclaration : variable.getScopes()) {
			os.print(".");
			os.print(scopeDeclaration.getScoper());
		}
	}

	@Override
	public void putParentheses(ParenthesesStatement parentheses, PrintStream os) {
		os.print("(");
		parentheses.getEnclosedExpression().write(this, os);
		os.print(")");
		for (ScopeDeclaration scopeDeclaration : parentheses.getScopes()) {
			os.print(".");
			os.print(scopeDeclaration.getScoper());
		}
	}

	@Override
	public void putOperatorExpression(FunctionCallStatement opfunc, PrintStream os) {
		List<ExpressionStatement> arguments = opfunc.getArguments();
		arguments.get(0).write(this, os);
		// operator[type] substring
		os.print(opfunc.getFunction().getName().substring(8));
		arguments.get(1).write(this, os);
		if (opfunc.isStandalone()) {
			os.println(";");
		}
	}

	@Override
	public void putFunctionCall(FunctionCallStatement functioncall, PrintStream os) {
		FunctionDeclaration sample2dfunc = shaders.resolveFunction("sample", "texture2D", "float2");
		List<ExpressionStatement> args = functioncall.getArguments();
		if (functioncall.getFunction().equals(sample2dfunc)) {
			args.get(0).write(this, os);
			os.print(".Sample(");
			args.get(0).write(this, os);
			os.print("_sampler, ");
			args.get(1).write(this, os);
			os.print(")");
		} else {
			os.print(functioncall.getFunction().getName());
			os.print("(");
			for (Iterator<ExpressionStatement> it = args.iterator(); it.hasNext();) {
				ExpressionStatement exp = it.next();
				exp.write(this, os);
				if (it.hasNext()) {
					os.print(",");
				}
			}
			os.print(")");
		}
		if (functioncall.isStandalone()) {
			os.println(";");
		} else {
			for (ScopeDeclaration scopeDeclaration : functioncall.getScopes()) {
				os.print(".");
				os.print(scopeDeclaration.getScoper());
			}
		}
	}

	@Override
	public void putBlockStatement(BlockStatement block, PrintStream os) {
		for (ShaderStatement stm : block.getSubStatements()) {
			stm.write(this, os);
		}
	}

	@Override
	public void putAssignment(AssignmentStatement assignment, PrintStream os) {
		os.print(assignment.getTarget().getName());
		os.print("=");
		assignment.getValueExpression().write(this, os);
		os.println(";");
	}

	@Override
	public void putIfControl(IfControlStatement ifControl, PrintStream os) {
		BlockStatement onfalse = ifControl.getOnfalse();

		os.print("if(");
		ifControl.getCondition().write(this, os);
		os.println("){");

		ifControl.getOntrue().write(this, os);
		if (onfalse != null) {
			os.println("}else{");
			onfalse.write(this, os);
		}
		os.println("}");
	}

	@Override
	public void putWhileControl(WhileControlStatement whileControl, PrintStream os) {
		os.print("while(");
		whileControl.getCondition().write(this, os);
		os.println(") {");
		whileControl.getBody().write(this, os);
		os.println("}");
	}

}
