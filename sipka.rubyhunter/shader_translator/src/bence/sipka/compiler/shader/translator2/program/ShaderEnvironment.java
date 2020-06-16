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
package bence.sipka.compiler.shader.translator2.program;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import bence.sipka.compiler.shader.translator2.program.elements.ShaderType;
import bence.sipka.compiler.shader.translator2.program.statements.FunctionDeclaration2;
import bence.sipka.compiler.shader.translator2.program.statements.ScopeDeclaration2;
import sipka.syntax.parser.model.statement.Statement;
import sipka.syntax.parser.model.statement.modifiable.ModifiableStatement;

public class ShaderEnvironment {
	private Collection<ShaderProgram2> shaders = new ArrayList<>();
	private Map<String, ShaderType> types = new HashMap<>();
	private Set<ScopeDeclaration2> scopeDeclarations = new HashSet<>();
	private Set<FunctionDeclaration2> functions = new HashSet<>();

	{
		/**
		 * Matrices: float[rowlen]x[columnlen]
		 */

		ShaderType tvoid = new ShaderType("void");

		ShaderType tfloat = new ShaderType("float");
		ShaderType tfloat4 = new ShaderType("Vector4F");
		ShaderType tfloat2 = new ShaderType("Vector2F");
		ShaderType tfloat3 = new ShaderType("Vector3F");

		ShaderType mat2 = new ShaderType("Matrix<2>");
		ShaderType mat3 = new ShaderType("Matrix<3>");
		ShaderType mat4 = new ShaderType("Matrix<4>");

		ShaderType tbool = new ShaderType("bool");

		ShaderType ttexture2d = new ShaderType("texture2D");

		types.put("void", tvoid);

		types.put("float", tfloat);
		types.put("float2", tfloat2);
		types.put("float3", tfloat3);
		types.put("float4", tfloat4);

		types.put("float2x2", mat2);
		types.put("float3x3", mat3);
		types.put("float4x4", mat4);
		types.put("mat2", mat2);
		types.put("mat3", mat3);
		types.put("mat4", mat4);

		types.put("bool", tbool);

		types.put("texture2D", ttexture2d);

		types.put("vertex_position", tfloat4);
		types.put("fragment_color", tfloat4);
		types.put("depth", tfloat);

		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", tfloat, tfloat));
		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", tfloat2, tfloat2));
		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", tfloat3, tfloat3));
		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", tfloat4, tfloat4));

		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", mat2, mat2));
		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", mat3, mat3));
		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", mat4, mat4));

		functions.add(FunctionDeclaration2.createOperator(tvoid, "=", tbool, tbool));

		functions.add(FunctionDeclaration2.createOperator(tfloat4, "*", tfloat4, mat4));
		functions.add(FunctionDeclaration2.createOperator(tfloat3, "*", tfloat3, mat3));
		functions.add(FunctionDeclaration2.createOperator(tfloat2, "*", tfloat2, mat2));

		functions.add(FunctionDeclaration2.createOperator(tfloat4, "*", mat4, tfloat4));
		functions.add(FunctionDeclaration2.createOperator(tfloat3, "*", mat3, tfloat3));
		functions.add(FunctionDeclaration2.createOperator(tfloat2, "*", mat2, tfloat2));

		functions.add(FunctionDeclaration2.createFunction(tfloat, "dot", tfloat4, tfloat4));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "dot", tfloat3, tfloat3));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "dot", tfloat2, tfloat2));

		functions.add(FunctionDeclaration2.createFunction(tfloat, "max", tfloat, tfloat));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "min", tfloat, tfloat));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "pow", tfloat, tfloat));

		functions.add(FunctionDeclaration2.createFunction(tfloat4, "normalize", tfloat4));
		functions.add(FunctionDeclaration2.createFunction(tfloat3, "normalize", tfloat3));
		functions.add(FunctionDeclaration2.createFunction(tfloat2, "normalize", tfloat2));

		functions.add(FunctionDeclaration2.createFunction(tfloat4, "cross", tfloat4, tfloat4));
		functions.add(FunctionDeclaration2.createFunction(tfloat3, "cross", tfloat3, tfloat3));
		functions.add(FunctionDeclaration2.createFunction(tfloat2, "cross", tfloat2, tfloat2));

		functions.add(FunctionDeclaration2.createFunction(tfloat, "length", tfloat4));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "length", tfloat3));
		functions.add(FunctionDeclaration2.createFunction(tfloat, "length", tfloat2));

		functions.add(FunctionDeclaration2.createFunction(tfloat4, "sample", ttexture2d, tfloat2));

		declareArithmeticOperatorsForType(tfloat);
		declareArithmeticOperatorsForType(tfloat2);
		declareArithmeticOperatorsForType(tfloat3);
		declareArithmeticOperatorsForType(tfloat4);

		declareArithmeticOperatorsForType(mat2);
		declareArithmeticOperatorsForType(mat3);
		declareArithmeticOperatorsForType(mat4);

		declareArithmeticOperators(mat2, tfloat, mat2);
		declareArithmeticOperators(mat2, mat2, tfloat);

		declareArithmeticOperators(mat3, tfloat, mat3);
		declareArithmeticOperators(mat3, mat3, tfloat);

		declareArithmeticOperators(mat4, tfloat, mat4);
		declareArithmeticOperators(mat4, mat4, tfloat);

		declareCompareOperators(tfloat, tbool);

		declareFloatArithmeticOperators(tfloat, tfloat2);
		declareFloatArithmeticOperators(tfloat, tfloat3);
		declareFloatArithmeticOperators(tfloat, tfloat4);

		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "x", tfloat));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "y", tfloat));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "z", tfloat));

		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "xx", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "xy", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "xz", tfloat2));

		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "yx", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "yy", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "yz", tfloat2));

		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "zx", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "zy", tfloat2));
		scopeDeclarations.add(new ScopeDeclaration2(tfloat3, "zz", tfloat2));

		addScopes(new ShaderType[] { null, tfloat, tfloat2, tfloat3, tfloat4 }, new String[] { "", "x", "y", "z", "w" });
		addScopes(new ShaderType[] { null, tfloat, tfloat2, tfloat3, tfloat4 }, new String[] { "", "r", "g", "b", "a" });
		addScopes(new ShaderType[] { null, tfloat, tfloat2, tfloat3, tfloat4 }, new String[] { "", "s", "t", "p", "q" });
	}

	private void addScopes(ShaderType[] types, String[] scopes) {
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

						ShaderType targettype = types[length];
						if (a < 4 && b < 4 && c < 4 && d < 4 && length < 4) {
							// doesnt contain w, q, or a
							// apply scope to tfloat 3
							scopeDeclarations.add(new ScopeDeclaration2(types[3], scp, targettype));
						}
						scopeDeclarations.add(new ScopeDeclaration2(types[4], scp, targettype));

					}
				}
			}
		}
	}

	private void declareArithmeticOperatorsForType(ShaderType type) {
		declareArithmeticOperators(type, type, type);
	}

	private void declareArithmeticOperators(ShaderType returntype, ShaderType left, ShaderType right) {
		functions.add(FunctionDeclaration2.createOperator(returntype, "*", left, right));
		functions.add(FunctionDeclaration2.createOperator(returntype, "/", left, right));
		functions.add(FunctionDeclaration2.createOperator(returntype, "+", left, right));
		functions.add(FunctionDeclaration2.createOperator(returntype, "-", left, right));
	}

	private void declareCompareOperators(ShaderType type, ShaderType tbool) {
		functions.add(FunctionDeclaration2.createOperator(tbool, "<", type, type));
		functions.add(FunctionDeclaration2.createOperator(tbool, "<=", type, type));
		functions.add(FunctionDeclaration2.createOperator(tbool, "==", type, type));
		functions.add(FunctionDeclaration2.createOperator(tbool, "!=", type, type));
		functions.add(FunctionDeclaration2.createOperator(tbool, ">=", type, type));
		functions.add(FunctionDeclaration2.createOperator(tbool, ">", type, type));
	}

	private void declareFloatArithmeticOperators(ShaderType tfloat, ShaderType other) {
		declareArithmeticOperators(other, tfloat, other);
		declareArithmeticOperators(other, other, tfloat);
	}

	public ShaderEnvironment() {
	}

	public ShaderEnvironment(ShaderEnvironment copy) {
		shaders.addAll(copy.shaders);
	}

	public void addShader(Statement stm) {
		shaders.add(new ShaderProgram2(this, ModifiableStatement.parse(stm)));
	}

	public ShaderType getType(String name) {
		return types.get(name);
	}
}
