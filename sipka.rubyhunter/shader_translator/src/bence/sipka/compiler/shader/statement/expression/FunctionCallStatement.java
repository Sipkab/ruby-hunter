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
package bence.sipka.compiler.shader.statement.expression;

import java.io.PrintStream;
import java.util.List;

import bence.sipka.compiler.shader.elements.FunctionDeclaration;
import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class FunctionCallStatement extends ExpressionStatement {
	private static final long serialVersionUID = 1L;

	protected FunctionDeclaration function;
	protected List<ExpressionStatement> arguments;
	protected boolean standalone;

	public FunctionCallStatement(FunctionDeclaration function, List<ExpressionStatement> arguments) {
		super(function.getReturnType());
		this.function = function;
		this.arguments = arguments;
	}

	public FunctionCallStatement(FunctionDeclaration function, List<ExpressionStatement> arguments, List<ScopeDeclaration> scopes) {
		super(function.getReturnType(), scopes);
		this.function = function;
		this.arguments = arguments;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		if (function.isOperator()) {
			translator.putOperatorExpression(this, os);
		} else {
			translator.putFunctionCall(this, os);
		}
	}

	public FunctionDeclaration getFunction() {
		return function;
	}

	public List<ExpressionStatement> getArguments() {
		return arguments;
	}

	public boolean isStandalone() {
		return standalone;
	}

	public void setStandalone(boolean standalone) {
		this.standalone = standalone;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((arguments == null) ? 0 : arguments.hashCode());
		result = prime * result + ((function == null) ? 0 : function.hashCode());
		result = prime * result + (standalone ? 1231 : 1237);
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
		FunctionCallStatement other = (FunctionCallStatement) obj;
		if (arguments == null) {
			if (other.arguments != null)
				return false;
		} else if (!arguments.equals(other.arguments))
			return false;
		if (function == null) {
			if (other.function != null)
				return false;
		} else if (!function.equals(other.function))
			return false;
		if (standalone != other.standalone)
			return false;
		return true;
	}

}
