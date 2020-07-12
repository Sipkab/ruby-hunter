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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.Stack;
import java.util.regex.Pattern;

import bence.sipka.compiler.shader.elements.AttributeDeclaration;
import bence.sipka.compiler.shader.elements.FunctionDeclaration;
import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.elements.TypeDeclaration;
import bence.sipka.compiler.shader.elements.UniformDeclaration;
import bence.sipka.compiler.shader.elements.VariableDeclaration;
import bence.sipka.compiler.shader.statement.BlockStatement;
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
import sipka.syntax.parser.model.statement.Statement;
import sipka.syntax.parser.model.statement.modifiable.ModifiableStatement;

public class ShaderResource implements Serializable {
	private static final long serialVersionUID = 1L;

	public static final int TYPE_VERTEX = 0;
	public static final int TYPE_FRAGMENT = 1;

	private final String uri;
	private final int type;
	private ShaderProgram definingProgram;

	private List<UniformDeclaration> uniforms = new ArrayList<>();
	private List<VariableDeclaration> inputVariables = new ArrayList<>();
	private List<VariableDeclaration> outputVariables = new ArrayList<>();

	private BlockStatement statements = new BlockStatement();

	private int referenceCount = 0;

	private ClassUrl classUrl;

	public ShaderResource(String uri, int type) {
		this.uri = uri;
		this.type = type;
		String cname = uri.replace(".", "_");
		this.classUrl = new ClassUrl(cname, "gen/shader/" + cname + ".h");
	}

	public ClassUrl getClassUrl() {
		return classUrl;
	}

	public String getUri() {
		return uri;
	}

	public void define(ShaderCollection shaders, Statement stm, ShaderProgram parent) {
		if (isDefined()) {
			throw new RuntimeException("Shader resource is already defined, with uri: " + uri);
		}
		definingProgram = parent;

		Stack<VariableDeclaration> varstack = new Stack<>();

		ModifiableStatement root = ModifiableStatement.parse(stm);

		takeShaderVariables(varstack, root, shaders);

		parseBlockStatements(varstack, root, statements, shaders);

	}

	public TypeDeclaration resolveType(ShaderCollection shaders, String name) {
		TypeDeclaration result = shaders.resolveType(name);
		if (result == null) {
			throw new RuntimeException("Type not found with name: " + name);
		}
		return result;
	}

	public FunctionDeclaration resolveFunction(ShaderCollection shaders, String name,
			List<ExpressionStatement> arguments) {
		List<TypeDeclaration> argtypes = new ArrayList<>(arguments.size());
		for (ExpressionStatement exp : arguments) {
			argtypes.add(exp.getExpressionType());
		}
		FunctionDeclaration result = shaders.resolveFunction(name, argtypes);
		if (result == null) {
			throw new RuntimeException("Function not found with name: " + name + " and arguments: " + argtypes);
		}
		return result;
	}

	public ScopeDeclaration resolveScope(ShaderCollection shaders, TypeDeclaration source, String scoper) {
		ScopeDeclaration result = shaders.resolveScope(source, scoper);
		if (result == null) {
			throw new RuntimeException("Scope not found from type: " + source + " with scope: " + scoper);
		}
		return result;
	}

	private void parseBlockStatements(Stack<VariableDeclaration> varstack, ModifiableStatement root,
			BlockStatement parent, ShaderCollection shaders) {
		for (ModifiableStatement child : root) {
			switch (child.getName()) {
				// case "assignment": {
				// String varname = child.getChildValue("var_name");
				// throwOnVariableNotExists(varstack, varname);
				//
				// ExpressionStatement exp = parseExpression(varstack, child.getChildNamed("assigned_value"));
				// VariableDeclaration foundvar = findVariableInStack(varstack, varname);
				//
				// if (!exp.getExpressionType().equals(foundvar.getType())) {
				// throw new RuntimeException("Assignment types doesnt match, var: " + foundvar.getType().getDeclaredName() + " " + foundvar.getName()
				// +
				// " = "
				// + exp.getExpressionType().getDeclaredName());
				// }
				// if (!foundvar.isAssignable()) {
				// throw new RuntimeException("Variable is not assignable: " + foundvar);
				// }
				// parent.addSubStatement(new AssignmentStatement(foundvar, exp));
				// break;
				// }
				case "variable_decl": {
					String typestring = child.getChildValue("var_type");
					VariableDeclaration decl = new VariableDeclaration(resolveType(shaders, typestring),
							child.getChildValue("var_name"));
					decl.setAttributes(takeAttributes(child.getChildNamed("attributes")));
					throwOnRedeclaration(varstack, decl);
					varstack.add(decl);

					ModifiableStatement initialization = child.getChildNamed("initial_value");
					if (initialization != null) {
						ExpressionStatement exp = parseExpression(varstack, initialization, shaders);

						if (!exp.getExpressionType().equals(decl.getType())) {
							throw new RuntimeException(
									"Assignment types doesnt match, var: " + decl.getType().getDeclaredName() + " "
											+ decl.getName() + " = " + exp.getExpressionType().getDeclaredName());
						}

						parent.addSubStatement(new VarDeclarationStatement(decl, exp));
					} else {
						parent.addSubStatement(new VarDeclarationStatement(decl));
					}
					break;
				}
				case "statement": {
					ExpressionStatement exp = parseExpression(varstack, child, shaders);
					parent.addSubStatement(exp);
					break;
				}
				case "if_statement": {
					ModifiableStatement elsestm = child.getChildNamed("else_body");

					ExpressionStatement condition = parseExpression(varstack, child.getChildNamed("if_condition"),
							shaders);
					if (!condition.getExpressionType().equals(resolveType(shaders, "bool"))) {
						throw new RuntimeException("If condition is not bool type");
					}
					BlockStatement ontrue = new BlockStatement();
					BlockStatement onfalse = null;

					parseBlockStatements(varstack, child.getChildNamed("if_body"), ontrue, shaders);

					if (elsestm != null) {
						onfalse = new BlockStatement();
						parseBlockStatements(varstack, elsestm, onfalse, shaders);
					}
					parent.addSubStatement(new IfControlStatement(condition, ontrue, onfalse));
					break;
				}
				case "while_statement": {
					ExpressionStatement condition = parseExpression(varstack, child.getChildNamed("while_condition"),
							shaders);
					if (!condition.getExpressionType().equals(resolveType(shaders, "bool"))) {
						throw new RuntimeException("While condition is not bool type");
					}
					BlockStatement body = new BlockStatement();
					parseBlockStatements(varstack, child.getChildNamed("while_body"), body, shaders);

					parent.addSubStatement(new WhileControlStatement(condition, body));
					break;
				}
				default: {
					break;
				}
			}
		}
	}

	private static void fixSingleOperator(ModifiableStatement opstm, ModifiableStatement block) {
		ModifiableStatement replacestm = new ModifiableStatement("operator_call", opstm.getValue(), null);
		ModifiableStatement left = new ModifiableStatement("op_left", "", null);
		ModifiableStatement right = new ModifiableStatement("op_right", "", null);

		left.add(opstm.getPreviousSibling());
		right.add(opstm.getNextSibling());

		replacestm.add(left);
		replacestm.add(right);

		block.replaceChild(opstm, replacestm);
	}

	public static final Pattern PRECEDENCE_OPERATOR_PATTERN = Pattern.compile("[*/]");

	private static void fixOperators(ModifiableStatement block) {
		List<ModifiableStatement> opchildren = block.getChildrenNamed("operator");
		opchildren.sort(new Comparator<ModifiableStatement>() {
			@Override
			public int compare(ModifiableStatement o1, ModifiableStatement o2) {
				return Integer.compare("*/+-=".indexOf(o1.getValue()), "*/+-=".indexOf(o2.getValue()));
			}
		});
		for (ModifiableStatement opstm : opchildren) {
			fixSingleOperator(opstm, block);
		}
	}

	private ExpressionStatement parseExpression(Stack<VariableDeclaration> varstack, ModifiableStatement expression,
			ShaderCollection shaders) {
		fixOperators(expression);

		ModifiableStatement child = expression.getChildAt(0);
		switch (child.getName()) {
			case "parentheses": {
				return new ParenthesesStatement(parseExpression(varstack, child, shaders));
			}
			case "operator_call": {
				ExpressionStatement left = parseExpression(varstack, child.getChildNamed("op_left"), shaders);
				ExpressionStatement right = parseExpression(varstack, child.getChildNamed("op_right"), shaders);
				String operatorType = child.getValue();
				List<ExpressionStatement> arguments = Arrays.asList(left, right);
				FunctionDeclaration resolved = resolveFunction(shaders, "operator" + operatorType, arguments);
				if (resolved == null) {
					throw new RuntimeException("Operator " + operatorType + " not found for types: "
							+ Arrays.asList(left.getExpressionType(), right.getExpressionType()));
				}
				FunctionCallStatement function = new FunctionCallStatement(resolved, arguments);
				function.setStandalone(expression.getName().equals("statement"));
				return function;
			}
			case "float_literal": {
				return new FloatLiteral(child.getValue(), resolveType(shaders, "float"));
			}
			case "int_literal": {
				// TODO itt nem float type, hanem int ha mar lesz
				return new IntegerLiteral(child.getValue(), resolveType(shaders, "float"));
			}
			case "bool_literal": {
				return new BoolLiteral(child.getValue(), resolveType(shaders, "bool"));
			}
			case "variable": {
				throwOnVariableNotExists(varstack, child.getValue());
				VariableDeclaration foundvar = findVariableInStack(varstack, child.getValue());
				List<ModifiableStatement> scopechildren = child.getChildrenNamed("scope");

				if (scopechildren.size() > 0) {
					List<ScopeDeclaration> scopes = new ArrayList<>();
					TypeDeclaration lasttype = foundvar.getType();
					for (ModifiableStatement scopestm : scopechildren) {
						ScopeDeclaration resolvedScope = resolveScope(shaders, lasttype, scopestm.getValue());
						if (resolvedScope == null) {
							throw new RuntimeException(
									"Failed to scope to: " + scopestm.getValue() + " on var: " + foundvar);
						}
						scopes.add(resolvedScope);
						lasttype = resolvedScope.getTargetType();
					}
					return new VariableExpression(foundvar, scopes);
				}
				return new VariableExpression(foundvar);
			}
			case "function_call": {
				String funcname = child.getChildValue("function_name");
				List<ModifiableStatement> scopechildren = child.getChildrenNamed("scope");

				List<ExpressionStatement> arguments = new ArrayList<>();
				for (ModifiableStatement argstm : child.getChildrenNamed("function_arg")) {
					arguments.add(parseExpression(varstack, argstm, shaders));
				}

				FunctionDeclaration foundfunc = resolveFunction(shaders, funcname, arguments);
				if (scopechildren.size() > 0) {
					List<ScopeDeclaration> scopes = new ArrayList<>();
					TypeDeclaration lasttype = foundfunc.getReturnType();
					for (ModifiableStatement scopestm : scopechildren) {
						ScopeDeclaration resolvedScope = resolveScope(shaders, lasttype, scopestm.getValue());
						if (resolvedScope == null) {
							throw new RuntimeException(
									"Failed to scope to: " + scopestm.getValue() + " on var: " + foundfunc);
						}
						scopes.add(resolvedScope);
						lasttype = resolvedScope.getTargetType();
					}
					FunctionCallStatement function = new FunctionCallStatement(foundfunc, arguments, scopes);
					return function;
				}

				FunctionCallStatement function = new FunctionCallStatement(foundfunc, arguments);
				function.setStandalone(expression.getName().equals("statement"));
				return function;
			}
			default: {
				throw new RuntimeException("Unknown child found while parsing shader: " + child);
			}
		}
	}

	private static List<AttributeDeclaration> takeAttributes(ModifiableStatement attrsstm) {
		if (attrsstm == null) {
			return Collections.emptyList();
		}
		ArrayList<AttributeDeclaration> result = new ArrayList<>();
		for (ModifiableStatement attr : attrsstm.getChildren()) {
			result.add(new AttributeDeclaration(attr.getChildValue("key"), attr.getChildValue("value")));
		}
		return result;
	}

	private void takeShaderVariables(Stack<VariableDeclaration> varstack, ModifiableStatement root,
			ShaderCollection shaders) {
		ClassUrl classurl = getClassUrl();
		Objects.requireNonNull(classurl, "thisclassurl");
		for (ModifiableStatement unistm; (unistm = root.getChildNamed("uniform_var")) != null; root
				.removeChild(unistm)) {
			UniformDeclaration unidecl = new UniformDeclaration(unistm.getChildValue("uniform_name"), classurl);
			for (ModifiableStatement varstm : unistm.getChildrenNamed("variable")) {
				VariableDeclaration decl = new VariableDeclaration(
						resolveType(shaders, varstm.getChildValue("var_type")), varstm.getChildValue("var_name"));
				decl.setAttributes(takeAttributes(varstm.getChildNamed("attributes")));
				decl.setAssignable(false);
				unidecl.addMember(decl);

				throwOnRedeclaration(varstack, decl);
				varstack.add(decl);
			}
			uniforms.add(unidecl);
		}

		for (ModifiableStatement varstm; (varstm = root.getChildNamed("input_var")) != null; root.removeChild(varstm)) {
			VariableDeclaration decl = new VariableDeclaration(resolveType(shaders, varstm.getChildValue("var_type")),
					varstm.getChildValue("var_name"));
			decl.setAttributes(takeAttributes(varstm.getChildNamed("attributes")));
			decl.setAssignable(false);
			throwOnRedeclaration(varstack, decl);
			inputVariables.add(decl);
			varstack.add(decl);
		}

		for (ModifiableStatement varstm; (varstm = root.getChildNamed("output_var")) != null;) {
			VariableDeclaration decl = new VariableDeclaration(resolveType(shaders, varstm.getChildValue("var_type")),
					varstm.getChildValue("var_name"));
			decl.setAttributes(takeAttributes(varstm.getChildNamed("attributes")));
			throwOnRedeclaration(varstack, decl);
			outputVariables.add(decl);
			varstack.add(decl);

			ModifiableStatement initialization = varstm.getChildNamed("initial_value");
			if (initialization == null) {
				root.removeChild(varstm);
			} else {
				varstm.setName("statement");
				varstm.removeFirstChildWithName("var_type");
				varstm.removeChild(initialization);

				ModifiableStatement variable = varstm.getChildrenNamed("var_name").get(0);
				variable.setName("variable");
				variable.add(varstm.getChildNamed("attributes"));

				varstm.insertAfter(variable, new ModifiableStatement("operator", "=", null));

				varstm.addChildren(initialization.getChildren());
			}

		}
	}

	private static VariableDeclaration findVariableInStack(Stack<VariableDeclaration> stack, String name) {
		for (VariableDeclaration vardecl : stack) {
			if (vardecl.getName().equals(name))
				return vardecl;
		}
		return null;
	}

	private static void throwOnRedeclaration(Stack<VariableDeclaration> stack, VariableDeclaration var) {
		throwOnRedeclaration(stack, var.getName());
	}

	private static void throwOnRedeclaration(Stack<VariableDeclaration> stack, String name) {
		VariableDeclaration found = findVariableInStack(stack, name);
		if (found != null) {
			throw new RuntimeException("Redeclaration of variable: " + found.getType() + " " + found.getName());
		}
	}

	private static void throwOnVariableNotExists(Stack<VariableDeclaration> stack, String name) {
		if (findVariableInStack(stack, name) == null) {
			throw new RuntimeException("Variable doesnt exist with name: " + name);
		}
	}

	public List<UniformDeclaration> getUniforms() {
		return uniforms;
	}

	public List<VariableDeclaration> getInputVariables() {
		return inputVariables;
	}

	public List<VariableDeclaration> getOutputVariables() {
		return outputVariables;
	}

	public int getType() {
		return type;
	}

	public BlockStatement getStatements() {
		return statements;
	}

	public boolean isDefined() {
		return definingProgram != null;
	}

	public ShaderProgram getDefiningProgram() {
		return definingProgram;
	}

	public void addReference() {
		++referenceCount;
	}

	public int getReferenceCount() {
		return referenceCount;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((classUrl == null) ? 0 : classUrl.hashCode());
		result = prime * result + ((inputVariables == null) ? 0 : inputVariables.hashCode());
		result = prime * result + ((outputVariables == null) ? 0 : outputVariables.hashCode());
		result = prime * result + ((statements == null) ? 0 : statements.hashCode());
		result = prime * result + type;
		result = prime * result + ((uniforms == null) ? 0 : uniforms.hashCode());
		result = prime * result + ((uri == null) ? 0 : uri.hashCode());
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
		ShaderResource other = (ShaderResource) obj;
		if (classUrl == null) {
			if (other.classUrl != null)
				return false;
		} else if (!classUrl.equals(other.classUrl))
			return false;
		if (inputVariables == null) {
			if (other.inputVariables != null)
				return false;
		} else if (!inputVariables.equals(other.inputVariables))
			return false;
		if (outputVariables == null) {
			if (other.outputVariables != null)
				return false;
		} else if (!outputVariables.equals(other.outputVariables))
			return false;
		if (statements == null) {
			if (other.statements != null)
				return false;
		} else if (!statements.equals(other.statements))
			return false;
		if (type != other.type)
			return false;
		if (uniforms == null) {
			if (other.uniforms != null)
				return false;
		} else if (!uniforms.equals(other.uniforms))
			return false;
		if (uri == null) {
			if (other.uri != null)
				return false;
		} else if (!uri.equals(other.uri))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "ShaderResource [" + (uri != null ? "uri=" + uri : "") + "]";
	}

}
