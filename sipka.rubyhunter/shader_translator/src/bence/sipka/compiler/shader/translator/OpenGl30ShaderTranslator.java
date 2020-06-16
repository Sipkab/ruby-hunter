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
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Set;
import java.util.regex.Pattern;

import bence.sipka.compiler.shader.ShaderCollection;
import bence.sipka.compiler.shader.ShaderCompilerTaskFactory;
import bence.sipka.compiler.shader.ShaderProgram;
import bence.sipka.compiler.shader.ShaderResource;
import bence.sipka.compiler.shader.elements.AttributeDeclaration;
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
import bence.sipka.compiler.source.TemplatedSource;
import saker.build.file.SakerDirectory;
import saker.build.file.SakerFile;
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.SerializableContentDescriptor;
import saker.build.file.path.SakerPath;
import saker.build.task.TaskContext;
import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayInputStream;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

public class OpenGl30ShaderTranslator extends ShaderTranslator {
	private ShaderCollection shaders;

	public OpenGl30ShaderTranslator() {
		super("OpenGl30");
	}

	@Override
	public synchronized void translate(TaskContext taskcontext, SakerDirectory sourcesdir, SakerDirectory shadersdir,
			ShaderCollection shadersparam, NavigableMap<String, SakerPath> assets) throws IOException {
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
		for (final ShaderResource shader : shadercollection) {
			if (!shader.isDefined())
				continue;

			SakerFile glslfile = new SakerFileBase(
					getUniqueName().toUpperCase() + "_SHADER_" + shader.getUri() + "_" + getUniqueName() + ".glsl") {
				@Override
				public ContentDescriptor getContentDescriptor() {
					return shader.getDefiningProgram().getDefiningFileContentDescriptor();
				}

				@Override
				public InputStream openInputStreamImpl() throws IOException {
					return new UnsyncByteArrayInputStream(getBytesImpl());
				}

				@Override
				public ByteArrayRegion getBytesImpl() {
					try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream();
							PrintStream out = new PrintStream(baos)) {
						translate(shader, out);
						return baos.toByteArrayRegion();
					}
				}

				@Override
				public String getContentImpl() throws IOException {
					UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream();
					try (PrintStream out = new PrintStream(baos)) {
						translate(shader, out);
						return baos.toString();
					}
				}

				@Override
				public void writeToStreamImpl(OutputStream os) throws IOException {
					PrintStream out = new PrintStream(os);
					translate(shader, out);
					out.flush();
				}
			};
			shadersdir.add(glslfile);
			SakerPath glslassetpath = taskcontext.getTaskWorkingDirectoryPath()
					.relativize(shaders.getDefiningFile(shader).getSakerPath().getParent().resolve(glslfile.getName()));
			String assetname = glslassetpath.toString();
			assets.put(assetname, glslfile.getSakerPath());
			// operator.getResourcesDirectory().addFile(asset, FileState.OVERRIDDEN);
			if (shader.getReferenceCount() > 0) {
				// write class header
				SourceModularFile shaderClassFile = getShaderClassFile(shader,
						shader.getClassUrl().getExactClassName() + ".h", glslfile, glslassetpath);
				shadergendir.add(shaderClassFile);
			}
		}
		for (ShaderProgram prog : shaders.getPrograms()) {
			if (prog.isCompleteProgram()) {
				prog.validate();
				SourceModularFile programClassFile = getProgramClassFile(prog, prog.getName() + ".h");
				shadergendir.add(programClassFile);
			}
		}

	}

	private static String shaderTypeToGlType(int shadertype) {
		switch (shadertype) {
			case ShaderResource.TYPE_FRAGMENT: {
				return "GL_FRAGMENT_SHADER";
			}
			case ShaderResource.TYPE_VERTEX: {
				return "GL_VERTEX_SHADER";
			}
			default: {
				throw new RuntimeException("invalid shader resource type: " + shadertype);
			}
		}
	}

	private String glTypeToRenderType(String gltype) {
		switch (gltype) {
			case "sampler2D": {
				return getUniqueName() + "Texture*";
			}
			default: {
				return glTypeToFrameworkType(gltype);
			}
		}
	}

	private static String glTypeToFrameworkType(String gltype) {
		switch (gltype) {
			case "float": {
				return "float";
			}
			case "vec2": {
				return "Vector2F";
			}
			case "vec3": {
				return "Vector3F";
			}
			case "vec4": {
				return "Vector4F";
			}
			case "mat2": {
				return "Matrix<2>";
			}
			case "mat3": {
				return "Matrix<3>";
			}
			case "mat4": {
				return "Matrix<4>";
			}
			case "sampler2D": {
				return "Resource<render::Texture>";
			}
			default: {
				throw new RuntimeException("Found no type conversion from gl type to framework type: " + gltype);
			}
		}
	}

	private static boolean passFrameworkTypeByValue(String frameworktype) {
		return frameworktype.endsWith("*");
	}

	private static String glTypeFloatSize(String gltype) {
		return "(sizeof(" + glTypeToFrameworkType(gltype) + ")/sizeof(float))";
	}

	private String glTypeToUniformSetFunction(String gltype, String location, String object) {
		switch (gltype) {
			case "float": {
				return "renderer->glUniform1f(" + location + ", " + object + ")";
			}
			case "vec2": {
				return "renderer->glUniform2f(" + location + ", " + object + ".x(), " + object + ".y())";
			}
			case "vec3": {
				return "renderer->glUniform3f(" + location + ", " + object + ".x(), " + object + ".y(), " + object
						+ ".z())";
			}
			case "vec4": {
				return "renderer->glUniform4f(" + location + ", " + object + ".x(), " + object + ".y(), " + object
						+ ".z(), " + object + ".w())";
			}
			case "mat3": {
				return "renderer->glUniformMatrix3fv(" + location + ", 1, GL_FALSE, " + object + ")";
			}
			case "mat4": {
				return "renderer->glUniformMatrix4fv(" + location + ", 1, GL_FALSE, " + object + ")";
			}
			case "sampler2D": {
				return "renderer->glUniform1i(" + location + ", static_cast<" + getUniqueName() + "Texture*>(" + object
						+ ")->bind())";
			}
			default: {
				throw new RuntimeException("Found no type conversion from gl type to unifrom set function: " + gltype);
			}
		}
	}

	private static void writeShaderUniformImplementation(UniformDeclaration u, PrintStream out) {
		String uname = u.getName();
		out.println("class " + uname + "_impl final: public " + uname + " { public:");
		out.println("virtual bool load() override { return true; }");
		out.println("virtual void free() override { }");
		out.println("virtual bool reload() override { free(); return load(); }");
		out.println("virtual void update() override { }");
		out.println("};");
		out.println("virtual " + uname + "_impl* createUniform_" + uname + "Impl() override { return new " + uname
				+ "_impl{};}");
	}

	private SourceModularFile getShaderClassFile(ShaderResource shader, String name, SakerFile asset,
			SakerPath glslPath) throws IOException {
		String classname = shader.getClassUrl().getExactClassName();

		String assetid = glslPath.toString().replace('.', '_').replaceAll("[/\\\\]", "::");

		Map<String, Object> valmap = new HashMap<>();
		valmap.put("shader_resource_h_guard", (getUniqueName() + classname).toUpperCase() + "_H_");
		valmap.put("shader_resource_h_shadertype", shaderTypeToGlType(shader.getType()));
		valmap.put("shader_resource_h_classname", classname);
		valmap.put("shader_resource_h_assetfileid", assetid);

		TemplatedSource source = new TemplatedSource(ShaderCompilerTaskFactory.descriptor::getInputStream,
				"gen/shader/" + getUniqueName().toLowerCase() + "/template_shader_resource.h").setValueMap(valmap)
						.setHandler(new TranslationHandler() {
							@Override
							public void replaceKey(String key, OutputStream os) throws IOException {
								PrintStream out = new PrintStream(os);
								switch (key) {
									case "shader_resource_h_includes": {
										break;
									}
									case "shader_resource_h_private_members": {
										break;
									}
									case "shader_resource_h_protected_members": {
										break;
									}
									case "shader_resource_h_public_members": {
										for (UniformDeclaration u : shader.getUniforms()) {
											writeShaderUniformImplementation(u, out);
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

	private SourceModularFile getProgramClassFile(ShaderProgram prog, String name) {
		final ShaderResource vertexShader = prog.getVertexShader();
		final ShaderResource fragmentShader = prog.getFragmentShader();

		String classname = prog.getName();

		Map<String, Object> valmap = new HashMap<>();
		valmap.put("shader_program_h_guard", (getUniqueName() + classname).toUpperCase() + "_H_");
		valmap.put("shader_program_h_classname", classname);
		valmap.put("shader_program_h_vertexshadertype", vertexShader.getClassUrl().getExactClassName());
		valmap.put("shader_program_h_fragmentshadertype", fragmentShader.getClassUrl().getExactClassName());

		TemplatedSource source = new TemplatedSource(ShaderCompilerTaskFactory.descriptor::getInputStream,
				"gen/shader/" + getUniqueName().toLowerCase() + "/template_shader_program.h").setValueMap(valmap)
						.setHandler(new TranslationHandler() {
							Set<String> uniformNames = prog.getUniformNames();
							List<UniformDeclaration> unifroms = prog.getUniforms();

							@Override
							public void replaceKey(String key, OutputStream os) throws IOException {
								PrintStream out = new PrintStream(os);
								switch (key) {
									case "shader_program_h_private_members": {
										for (String name : uniformNames) {
											out.println("GLint uniform_" + name + ";");
										}
										for (VariableDeclaration in : vertexShader.getInputVariables()) {
											out.println("GLint attr_" + in.getName() + ";");
										}
										// Map<String, Object> hpmmap = new HashMap<>();
										// for (VariableDeclaration in : vertexShader.getInputVariables()) {
										// hpmmap.put("has_public_member_name", in.getName());
										// CppSourceTranslator.translate("has_public_member.cpp", out, hpmmap, null);
										// }
										break;
									}
									case "shader_program_h_protected_members": {
										break;
									}
									case "shader_program_h_public_members": {
										for (UniformDeclaration u : unifroms) {
											// out.printlnPostInc("class " + u.getName() + " {");
											// out.println("public:");
											// SeparatorBuffer constructorargs = new SeparatorBuffer(", ");
											// for (VariableDeclaration m : u.getMembers()) {
											// String fwtype = glTypeToFrameworkType(m.getType().getDeclaredName());
											// String rendertype = glTypeToRenderType(m.getType().getDeclaredName());
											// String name = m.getName();
											//
											// out.println(rendertype + " " + name + ";");
											// constructorargs.add(fwtype + " " + (passFrameworkTypeByValue(fwtype) ? "" : "const &") + name);
											// }
											//
											// out.printlnPostInc("void set(" + constructorargs + ") {");
											// for (VariableDeclaration v : u.getMembers()) {
											// String rendertype = glTypeToRenderType(v.getType().getDeclaredName());
											// String fwtype = glTypeToFrameworkType(v.getType().getDeclaredName());
											// if (rendertype.equals(fwtype)) {
											// out.println("this->" + v.getName() + " = " + v.getName() + ";");
											// } else {
											// out.println("this->" + v.getName() + " = static_cast<" + rendertype + ">(" + v.getName() + ");");
											// }
											// }
											// out.printlnPreDec("}");
											//
											// out.printlnPreDec("};");

											out.println("virtual void set(" + u.getName() + "& u) override {");
											out.println("ASSERT(isInUse()) << \"Program is not in use\";");
											for (VariableDeclaration m : u.getMembers()) {
												out.println(glTypeToUniformSetFunction(m.getType().getDeclaredName(),
														"uniform_" + m.getName(),
														"static_cast<" + getUniqueName()
																+ u.getParentShader().getClassUrl().getExactClassName()
																+ "::" + u.getName() + "_impl&>(u)." + m.getName())
														+ ";");
												out.println("CHECK_GL_ERROR()");
											}
											out.println("}");

										}

										break;
									}
									case "shader_program_h_program_created": {
										// query uniforms, attributes
										for (String u : uniformNames) {
											out.println("uniform_" + u + " = renderer->glGetUniformLocation(program, \""
													+ u + "\"); CHECK_GL_ERROR();");
										}
										for (VariableDeclaration in : vertexShader.getInputVariables()) {
											out.println("attr_" + in.getName()
													+ " = renderer->glGetAttribLocation(program, \"" + in.getName()
													+ "\"); CHECK_GL_ERROR();");
										}
										break;
									}
									case "shader_program_h_program_onuseprogram": {
										List<VariableDeclaration> inputs = vertexShader.getInputVariables();
										if (inputs.size() > 0) {
											Iterator<VariableDeclaration> it = inputs.iterator();
											VariableDeclaration in = it.next();
											out.println(
													"auto first = enableVertexAttribArray(attr_" + in.getName() + ");");
											for (; it.hasNext();) {
												in = it.next();
												out.println("enableVertexAttribArray(attr_" + in.getName() + ");");
											}
											out.println("disableStartingAt(first);");
										}

										break;
									}
									case "shader_program_h_inputlayout_activate": {

										List<VariableDeclaration> inputs = vertexShader.getInputVariables();
										for (int i = 0; i < inputs.size(); i++) {
											final VariableDeclaration in = inputs.get(i);
											final String declaredtype = in.getType().getDeclaredName();
											final String inname = in.getName();

											out.println("ASSERT(inputs[" + i + "].bufferIndex < bufferCount);");
											out.println(
													"static_cast<" + getUniqueName() + "VertexBuffer*>(buffers[inputs["
															+ i + "].bufferIndex])->activate();");
											out.println("program->renderer->glVertexAttribPointer(program->attr_"
													+ inname + ", " + glTypeFloatSize(declaredtype)
													+ ", GL_FLOAT, GL_FALSE, strides[inputs[" + i
													+ "].bufferIndex], (void*)(size_t)inputs[" + i
													+ "].offset); CHECK_GL_ERROR_REND(program->renderer);");
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

	private void replaceTypes() {
		Pattern tovec = Pattern.compile("float[2-4]");
		Pattern tomat = Pattern.compile("float[2-4]x[2-4]");
		Pattern textures = Pattern.compile("texture[2-3]D");
		Pattern elsetypes = Pattern.compile("void|bool|float|" + ShaderCollection.getVertexPositionOutputTypeName()
				+ "|" + ShaderCollection.getFragmentColorOutputTypeName());

		for (TypeDeclaration type : new HashSet<>(shaders.getBuiltinTypes().values())) {
			type = type.getRealType();
			String typename = type.getDeclaredName();
			if (tovec.matcher(typename).matches()) {
				type.setName(typename.replace("float", "vec"));
			} else if (tomat.matcher(typename).matches()) {
				if (typename.equals("float2x2") || typename.equals("float3x3") || typename.equals("float4x4")) {
					type.setName(typename.substring(0, 6).replace("float", "mat"));
				} else {
					type.setName(typename.replace("float", "mat"));
				}
			} else if (textures.matcher(typename).matches()) {
				type.setName(typename.replace("texture", "sampler"));
			} else {
				if (!elsetypes.matcher(typename).matches()) {
					throw new RuntimeException("Unrecognized shared type: " + typename);
				}
			}
		}
		shaders.resolveFunction("sample", shaders.resolveType("texture2D"), shaders.resolveType("float2"))
				.setName("texture2D");

	}

	private void analyze(ShaderResource shader) {
		String searchType;
		String replaceName;

		TypeDeclaration depthtype = shaders.resolveType("depth");

		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			searchType = ShaderCollection.getVertexPositionOutputTypeName();
			replaceName = "gl_Position";
		} else if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
			searchType = ShaderCollection.getFragmentColorOutputTypeName();
			replaceName = "gl_FragColor";
		} else {
			throw new RuntimeException("Output variable not found on shader: " + shader.getUri());
		}

		boolean had = false;
		boolean haddepth = false;
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
			} else if (o.getType().getDisplayName().equals(depthtype.getDisplayName())) {
				// do not output depth variable
				if (haddepth) {
					throw new RuntimeException("depth variable declared more than once");
				}
				if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
					throw new RuntimeException("Fragment shader cannot have depth output variable");
				}
				haddepth = true;
				continue;
			}
		}

		haddepth = false;
		for (Iterator<VariableDeclaration> it = shader.getInputVariables().iterator(); it.hasNext();) {
			VariableDeclaration in = it.next();
			if (in.getType().getDisplayName().equals(depthtype.getDisplayName())) {
				if (haddepth) {
					throw new RuntimeException("depth variable declared more than once");
				}
				if (shader.getType() == ShaderResource.TYPE_VERTEX) {
					throw new RuntimeException("Vertex shader cannot have depth input variable");
				}
				haddepth = true;
				continue;
			}
		}
	}

	private static String getVarPrecision(List<AttributeDeclaration> attrs) {
		// TODO ignore for now
		// for (AttributeDeclaration attr : attrs) {
		// if (attr.getKey().equals("fp_precision")) {
		// switch (attr.getValue()) {
		// case "low": {
		// return "lowp ";
		// }
		// case "medium": {
		// return "mediump ";
		// }
		// case "high": {
		// return "highp ";
		// }
		// default: {
		// break;
		// }
		// }
		// }
		// }
		return "";
	}

	private void translate(ShaderResource shader, PrintStream os) {
		List<VariableDeclaration> uniforms = new ArrayList<>();
		List<VariableDeclaration> inputs = shader.getInputVariables();
		List<VariableDeclaration> outputs = shader.getOutputVariables();
		for (UniformDeclaration uniform : shader.getUniforms()) {
			uniforms.addAll(uniform.getMembers());
		}
		// uniforms.sort((o1, o2) -> o1.getName().compareTo(o2.getName()));
		// inputs.sort((o1, o2) -> o1.getName().compareTo(o2.getName()));
		// outputs.sort((o1, o2) -> o1.getName().compareTo(o2.getName()));

		if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
			// TODO ignore for now
			// os.println("precision highp float;");
		}
		os.println("#version 110");
		os.println();
		uniforms.forEach(u -> os.println("uniform " + getVarPrecision(u.getAttributes()) + u.getType().getDeclaredName()
				+ " " + u.getName() + ";"));
		os.println();

		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			inputs.forEach(i -> os.println("attribute " + getVarPrecision(i.getAttributes())
					+ i.getType().getDeclaredName() + " " + i.getName() + ";"));
			os.println();
			for (VariableDeclaration o : outputs) {
				os.println("varying " + getVarPrecision(o.getAttributes()) + o.getType().getDeclaredName() + " "
						+ o.getName() + ";");
			}
			os.println();
		} else if (shader.getType() == ShaderResource.TYPE_FRAGMENT) {
			inputs.forEach(i -> os.println("varying " + getVarPrecision(i.getAttributes())
					+ i.getType().getDeclaredName() + " " + i.getName() + ";"));
			os.println();
		}

		os.println("void main(){");
		putBlockStatement(shader.getStatements(), os);
		TypeDeclaration depthtype = shaders.resolveType("depth");
		if (shader.getType() == ShaderResource.TYPE_VERTEX) {
			// has depth variable output?
			for (VariableDeclaration o : shader.getOutputVariables()) {
				if (o.getType().equals(depthtype)) {
					os.println(o.getName() + "=" + "gl_Position.z/gl_Position.w / 2.0 + 0.5;");
					break;
				}
			}
		}
		os.println("}");
		os.write(0);
	}

	@Override
	public void putVariableDeclaration(VarDeclarationStatement vardecl, PrintStream os) {
		ExpressionStatement initialization = vardecl.getInitialization();
		VariableDeclaration declaration = vardecl.getDeclaration();
		os.print(getVarPrecision(declaration.getAttributes()) + declaration.getType().getDeclaredName() + " "
				+ declaration.getName());
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
		os.print(functioncall.getFunction().getName());
		os.print("(");
		List<ExpressionStatement> args = functioncall.getArguments();
		for (Iterator<ExpressionStatement> it = args.iterator(); it.hasNext();) {
			ExpressionStatement exp = it.next();
			exp.write(this, os);
			if (it.hasNext()) {
				os.print(",");
			}
		}
		os.print(")");
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
