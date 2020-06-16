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

import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.elements.VariableDeclaration;
import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class VariableExpression extends ExpressionStatement {
	private static final long serialVersionUID = 1L;

	private VariableDeclaration variable;

	public VariableExpression(VariableDeclaration variable) {
		super(variable.getType());
		this.variable = variable;
	}

	public VariableExpression(VariableDeclaration variable, List<ScopeDeclaration> scopes) {
		super(variable.getType(), scopes);
		this.variable = variable;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putVariable(this, os);
	}

	public VariableDeclaration getVariable() {
		return variable;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((variable == null) ? 0 : variable.hashCode());
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
		VariableExpression other = (VariableExpression) obj;
		if (variable == null) {
			if (other.variable != null)
				return false;
		} else if (!variable.equals(other.variable))
			return false;
		return true;
	}

}
