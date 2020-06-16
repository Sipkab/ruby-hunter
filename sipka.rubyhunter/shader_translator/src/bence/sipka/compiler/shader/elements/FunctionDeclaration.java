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
package bence.sipka.compiler.shader.elements;

import java.io.Serializable;
import java.util.Arrays;
import java.util.List;

public class FunctionDeclaration implements Serializable {
	private static final long serialVersionUID = 1L;

	private final List<TypeDeclaration> arguments;
	private Object name;
	private final TypeDeclaration returnType;
	private boolean builtin;

	public static FunctionDeclaration createBuiltin(TypeDeclaration returnType, String name, TypeDeclaration... args) {
		FunctionDeclaration result = new FunctionDeclaration(returnType, name, Arrays.asList(args));
		result.builtin = true;
		return result;
	}

	public static FunctionDeclaration createOperator(TypeDeclaration returnType, String operatorType, TypeDeclaration... args) {
		FunctionDeclaration result = new FunctionDeclaration(returnType, "operator" + operatorType, Arrays.asList(args));
		result.builtin = true;
		return result;
	}

	public static FunctionDeclaration createConstructor(TypeDeclaration type, TypeDeclaration... args) {
		FunctionDeclaration result = new FunctionDeclaration(type, type, Arrays.asList(args));
		result.builtin = true;
		return result;
	}

	private FunctionDeclaration(TypeDeclaration returnType, Object name, List<TypeDeclaration> arguments) {
		this.name = name;
		this.returnType = returnType;
		this.arguments = arguments;
	}

	public FunctionDeclaration(TypeDeclaration returnType, String name, List<TypeDeclaration> arguments) {
		this(returnType, (Object) name, arguments);
	}

	public String getName() {
		if (name instanceof TypeDeclaration) {
			return ((TypeDeclaration) name).getDeclaredName();
		}
		return name.toString();
	}

	public void setName(String name) {
		this.name = name;
	}

	public boolean isBuiltin() {
		return builtin;
	}

	public boolean isOperator() {
		return name instanceof String && ((String) name).startsWith("operator");
	}

	public List<TypeDeclaration> getArguments() {
		return arguments;
	}

	public TypeDeclaration getReturnType() {
		return returnType;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((arguments == null) ? 0 : arguments.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
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
		FunctionDeclaration other = (FunctionDeclaration) obj;
		if (arguments == null) {
			if (other.arguments != null)
				return false;
		} else if (!arguments.equals(other.arguments))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "FunctionDeclaration [" + (returnType != null ? "returnType=" + returnType + ", " : "") + (name != null ? "name=" + name + ", " : "")
				+ (arguments != null ? "arguments=" + arguments : "") + "]";
	}

}
