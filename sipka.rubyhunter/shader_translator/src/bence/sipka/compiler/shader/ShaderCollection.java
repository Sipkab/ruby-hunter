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
package bence.sipka.compiler.shader;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Set;
import java.util.TreeMap;

import bence.sipka.compiler.shader.elements.FunctionDeclaration;
import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.elements.TypeDeclaration;
import saker.build.file.SakerFile;

public class ShaderCollection implements DeepCopyAble {
	private static final long serialVersionUID = 1L;

	private Map<String, SakerFile> programDefiningFiles = new TreeMap<>();
	private Contents contents = new Contents();

	private static class Contents implements DeepCopyAble {
		private static final long serialVersionUID = 1L;

		NavigableMap<String, ShaderResource> shaders = new TreeMap<>();
		NavigableMap<String, ShaderProgram> programs = new TreeMap<>();

		// TODO itt nem hash-elos dolgokat hasznaljunk, mert az elemek hash-je valtozhat
		Set<ScopeDeclaration> scopeDeclarations = new HashSet<>();
		NavigableMap<String, TypeDeclaration> builtinTypes = new TreeMap<>();
		Set<FunctionDeclaration> builtinFunctions = new HashSet<>();
	}

	public ShaderCollection() {

		TypeDeclaration tvoid = new TypeDeclaration("void", "void");

		/**
		 * Matrices: float[rowlen]x[columnlen]
		 */

		TypeDeclaration tfloat = new TypeDeclaration("float", "float");
		TypeDeclaration tfloat2 = new TypeDeclaration("float2", "Vector2F");
		TypeDeclaration tfloat3 = new TypeDeclaration("float3", "Vector3F");
		TypeDeclaration tfloat4 = new TypeDeclaration("float4", "Vector4F");
		// TODO invalid framework types

		TypeDeclaration tfloat2x2 = new TypeDeclaration("float2x2", "Matrix<2>");
		TypeDeclaration tfloat3x2 = new TypeDeclaration("float3x2", "");
		TypeDeclaration tfloat4x2 = new TypeDeclaration("float4x2", "");

		TypeDeclaration tfloat2x3 = new TypeDeclaration("float2x3", "");
		TypeDeclaration tfloat3x3 = new TypeDeclaration("float3x3", "Matrix<3>");
		TypeDeclaration tfloat4x3 = new TypeDeclaration("float4x3", "");

		TypeDeclaration tfloat2x4 = new TypeDeclaration("float2x4", "");
		TypeDeclaration tfloat3x4 = new TypeDeclaration("float3x4", "");
		TypeDeclaration tfloat4x4 = new TypeDeclaration("float4x4", "Matrix<4>");

		TypeDeclaration tbool = new TypeDeclaration("bool", "bool");

		TypeDeclaration ttexture2D = new TypeDeclaration("texture2D", "render::Texture*");
		TypeDeclaration ttexture3D = new TypeDeclaration("texture3D", "");

		putBuiltinType(new TypeDeclaration("vertex_position", tfloat4));
		putBuiltinType(new TypeDeclaration("fragment_color", tfloat4));
		putBuiltinType(new TypeDeclaration("depth", tfloat));

		putBuiltinType(new TypeDeclaration("mat2", tfloat2x2));
		putBuiltinType(new TypeDeclaration("mat3", tfloat3x3));
		putBuiltinType(new TypeDeclaration("mat4", tfloat4x4));

		putBuiltinType(tvoid);

		putBuiltinType(tfloat);
		putBuiltinType(tfloat2);
		putBuiltinType(tfloat3);
		putBuiltinType(tfloat4);

		putBuiltinType(tfloat2x2);
		putBuiltinType(tfloat2x3);
		putBuiltinType(tfloat2x4);

		putBuiltinType(tfloat3x2);
		putBuiltinType(tfloat3x3);
		putBuiltinType(tfloat3x4);

		putBuiltinType(tfloat4x2);
		putBuiltinType(tfloat4x3);
		putBuiltinType(tfloat4x4);

		putBuiltinType(tbool);

		putBuiltinType(ttexture2D);
		putBuiltinType(ttexture3D);

		// float numeric
		declareArithmeticOperatorsForType(tfloat);
		declareArithmeticOperatorsForType(tfloat2);
		declareArithmeticOperatorsForType(tfloat3);
		declareArithmeticOperatorsForType(tfloat4);

		declareArithmeticOperatorsForType(tfloat2x2);
		declareArithmeticOperatorsForType(tfloat3x3);
		declareArithmeticOperatorsForType(tfloat4x4);

		declareArithmeticOperators(tfloat2x2, tfloat, tfloat2x2);
		declareArithmeticOperators(tfloat2x2, tfloat2x2, tfloat);

		declareArithmeticOperators(tfloat3x3, tfloat, tfloat3x3);
		declareArithmeticOperators(tfloat3x3, tfloat3x3, tfloat);

		declareArithmeticOperators(tfloat4x4, tfloat, tfloat4x4);
		declareArithmeticOperators(tfloat4x4, tfloat4x4, tfloat);

		declareCompareOperators(tfloat, tbool);

		declareFloatArithmeticOperators(tfloat, tfloat2);
		declareFloatArithmeticOperators(tfloat, tfloat3);
		declareFloatArithmeticOperators(tfloat, tfloat4);

		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat2, tfloat2));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat3, tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat4, tfloat4));

		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat2x2, tfloat2x2));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat3x3, tfloat3x3));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tfloat4x4, tfloat4x4));

		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tvoid, "=", tbool, tbool));

		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat4, "*", tfloat4, tfloat4x4));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat3, "*", tfloat3, tfloat3x3));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat2, "*", tfloat2, tfloat2x2));

		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat4, "*", tfloat4x4, tfloat4));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat3, "*", tfloat3x3, tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tfloat2, "*", tfloat2x2, tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "dot", tfloat4, tfloat4));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "dot", tfloat3, tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "dot", tfloat2, tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "max", tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "min", tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "pow", tfloat, tfloat));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat4, "normalize", tfloat4));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat3, "normalize", tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat2, "normalize", tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat4, "cross", tfloat4, tfloat4));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat3, "cross", tfloat3, tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat2, "cross", tfloat2, tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "length", tfloat4));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "length", tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat, "length", tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createBuiltin(tfloat4, "sample", ttexture2D, tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat, tfloat));

		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat2, tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat2, tfloat2));

		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat3, tfloat, tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat3, tfloat2, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat3, tfloat, tfloat2));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat3, tfloat3));

		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat, tfloat, tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat2, tfloat, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat, tfloat2, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat, tfloat, tfloat2));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat2, tfloat2));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat3, tfloat));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat, tfloat3));
		contents.builtinFunctions.add(FunctionDeclaration.createConstructor(tfloat4, tfloat4));

		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "x", tfloat));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "y", tfloat));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "z", tfloat));

		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "xx", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "xy", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "xz", tfloat2));

		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "yx", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "yy", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "yz", tfloat2));

		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "zx", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "zy", tfloat2));
		contents.scopeDeclarations.add(new ScopeDeclaration(tfloat3, "zz", tfloat2));

		addScopes(new TypeDeclaration[] { null, tfloat, tfloat2, tfloat3, tfloat4 },
				new String[] { "", "x", "y", "z", "w" });
		addScopes(new TypeDeclaration[] { null, tfloat, tfloat2, tfloat3, tfloat4 },
				new String[] { "", "r", "g", "b", "a" });
		addScopes(new TypeDeclaration[] { null, tfloat, tfloat2, tfloat3, tfloat4 },
				new String[] { "", "s", "t", "p", "q" });
	}

	@SuppressWarnings("unchecked")
	@Override
	public <T extends DeepCopyAble> T deepCopy() {
		ShaderCollection result = new ShaderCollection();
		result.programDefiningFiles.putAll(this.programDefiningFiles);
		result.contents = this.contents.deepCopy();
		return (T) result;
	}

	private void putBuiltinType(TypeDeclaration type) {
		contents.builtinTypes.put(type.getDisplayName(), type);
	}

	private void addScopes(TypeDeclaration[] types, String[] scopes) {
		for (int a = 0; a < scopes.length; a++) {
			String first = scopes[a];
			for (int b = 0; b < scopes.length; b++) {
				String second = scopes[b];
				for (int c = 0; c < scopes.length; c++) {
					String third = scopes[c];
					for (int d = 0; d < scopes.length; d++) {
						String fourth = scopes[d];

						String scp = first + second + third + fourth;

						int length = scp.length();
						if (length == 0)
							continue;

						TypeDeclaration targettype = types[length];
						if (a < 4 && b < 4 && c < 4 && d < 4 && length < 4) {
							// doesnt contain w, q, or a
							// apply scope to tfloat 3
							contents.scopeDeclarations.add(new ScopeDeclaration(types[3], scp, targettype));
						}
						contents.scopeDeclarations.add(new ScopeDeclaration(types[4], scp, targettype));

					}
				}
			}
		}
	}

	private void declareArithmeticOperatorsForType(TypeDeclaration type) {
		declareArithmeticOperators(type, type, type);
	}

	private void declareArithmeticOperators(TypeDeclaration returntype, TypeDeclaration left, TypeDeclaration right) {
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(returntype, "*", left, right));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(returntype, "/", left, right));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(returntype, "+", left, right));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(returntype, "-", left, right));
	}

	private void declareCompareOperators(TypeDeclaration type, TypeDeclaration tbool) {
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, "<", type, type));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, "<=", type, type));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, "==", type, type));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, "!=", type, type));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, ">=", type, type));
		contents.builtinFunctions.add(FunctionDeclaration.createOperator(tbool, ">", type, type));
	}

	private void declareFloatArithmeticOperators(TypeDeclaration tfloat, TypeDeclaration other) {
		declareArithmeticOperators(other, tfloat, other);
		declareArithmeticOperators(other, other, tfloat);
	}

	public static String getVertexPositionOutputTypeName() {
		return "vertex_position";
	}

	public static String getFragmentColorOutputTypeName() {
		return "fragment_color";
	}

	public TypeDeclaration resolveType(String name) {
		return contents.builtinTypes.get(name);
	}

	public FunctionDeclaration resolveFunction(String name, TypeDeclaration... argtypes) {
		return resolveFunction(name, Arrays.asList(argtypes));
	}

	public FunctionDeclaration resolveFunction(String name, String... argtypes) {
		TypeDeclaration[] resolved = new TypeDeclaration[argtypes.length];
		for (int i = 0; i < argtypes.length; i++) {
			resolved[i] = resolveType(argtypes[i]);
			if (resolved[i] == null) {
				return null;
			}
		}
		return resolveFunction(name, resolved);
	}

	public FunctionDeclaration resolveFunction(String name, List<TypeDeclaration> argtypes) {
		for (FunctionDeclaration func : contents.builtinFunctions) {
			if (func.getName().equals(name) && func.getArguments().equals(argtypes)) {
				return func;
			}
		}
		return null;
	}

	public FunctionDeclaration resolveOperator(String operatorType, List<TypeDeclaration> argtypes) {
		String name = "operator" + operatorType;
		for (FunctionDeclaration func : contents.builtinFunctions) {
			if (func.getName().equals(name) && func.getArguments().equals(argtypes)) {
				return func;
			}
		}
		return null;
	}

	public FunctionDeclaration resolveOperator(String operatorType, String... argtypes) {
		TypeDeclaration[] resolved = new TypeDeclaration[argtypes.length];
		for (int i = 0; i < argtypes.length; i++) {
			resolved[i] = resolveType(argtypes[i]);
			if (resolved[i] == null) {
				return null;
			}
		}
		return resolveOperator(operatorType, Arrays.asList(resolved));
	}

	public ScopeDeclaration resolveScope(TypeDeclaration source, String scoper) {
		for (ScopeDeclaration sd : contents.scopeDeclarations) {
			if (sd.getSourceType().equals(source) && sd.getScoper().equals(scoper)) {
				return sd;
			}
		}
		return null;
	}

	public Set<ScopeDeclaration> getScopeDeclarations() {
		return contents.scopeDeclarations;
	}

	public Map<String, TypeDeclaration> getBuiltinTypes() {
		return contents.builtinTypes;
	}

	public Set<FunctionDeclaration> getBuiltinFunctions() {
		return contents.builtinFunctions;
	}

	public void addProgram(ShaderProgram program) {
		if (contents.programs.containsKey(program.getName())) {
			throw new RuntimeException("Shader program already defined: " + contents.programs.get(program.getName()));
		}
		contents.programs.put(program.getName(), program);
	}

	public ShaderResource getShaderResourceForUri(String uri, int type) {
		ShaderResource result = contents.shaders.get(uri);
		if (result == null) {
			result = new ShaderResource(uri, type);
			contents.shaders.put(uri, result);
		}
		return result;
	}

	public Collection<ShaderResource> getShaders() {
		return contents.shaders.values();
	}

	public Collection<ShaderProgram> getPrograms() {
		return contents.programs.values();
	}

	public void setDefiningFile(ShaderProgram program, SakerFile f) {
		program.setFile(f);
		programDefiningFiles.put(program.getName(), f);
	}

	public SakerFile getDefiningFile(ShaderResource res) {
		return programDefiningFiles.get(res.getDefiningProgram().getName());
	}

}
