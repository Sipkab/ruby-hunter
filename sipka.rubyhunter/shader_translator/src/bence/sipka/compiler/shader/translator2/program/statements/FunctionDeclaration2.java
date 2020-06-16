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
package bence.sipka.compiler.shader.translator2.program.statements;

import java.util.ArrayList;
import java.util.List;

import bence.sipka.compiler.shader.translator2.program.elements.ShaderType;

public class FunctionDeclaration2 {
	private List<ShaderType> argumentTypes = new ArrayList<>();
	private String name;
	private ShaderType returnType;

	public static FunctionDeclaration2 createOperator(ShaderType returnType, String operatorType, ShaderType... args) {
		return new FunctionDeclaration2(returnType, "operator" + operatorType, args);
	}

	public static FunctionDeclaration2 createConstructor(ShaderType type, ShaderType... args) {
		return new FunctionDeclaration2(type, type.getName(), args);
	}

	public static FunctionDeclaration2 createFunction(ShaderType returnType, String name, ShaderType... args) {
		return new FunctionDeclaration2(returnType, name, args);
	}

	public FunctionDeclaration2(ShaderType returnType, String name, ShaderType... arguments) {
		this.name = name;
		this.returnType = returnType;
		for (ShaderType a : arguments) {
			argumentTypes.add(a);
		}
	}

	public final List<ShaderType> getArgumentTypes() {
		return argumentTypes;
	}

	public final String getName() {
		return name;
	}

	public final void setName(String name) {
		this.name = name;
	}

	public final ShaderType getReturnType() {
		return returnType;
	}

	public final void setReturnType(ShaderType returnType) {
		this.returnType = returnType;
	}

}
