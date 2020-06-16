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
package bence.sipka.compiler.shader.statement;

import java.io.PrintStream;

import bence.sipka.compiler.shader.elements.VariableDeclaration;
import bence.sipka.compiler.shader.statement.expression.ExpressionStatement;
import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class AssignmentStatement extends ShaderStatement {
	private static final long serialVersionUID = 1L;

	private VariableDeclaration target;
	private ExpressionStatement value;

	public AssignmentStatement(VariableDeclaration var, ExpressionStatement value) {
		this.target = var;
		this.value = value;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putAssignment(this, os);
	}

	public VariableDeclaration getTarget() {
		return target;
	}

	public ExpressionStatement getValueExpression() {
		return value;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((target == null) ? 0 : target.hashCode());
		result = prime * result + ((value == null) ? 0 : value.hashCode());
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
		AssignmentStatement other = (AssignmentStatement) obj;
		if (target == null) {
			if (other.target != null)
				return false;
		} else if (!target.equals(other.target))
			return false;
		if (value == null) {
			if (other.value != null)
				return false;
		} else if (!value.equals(other.value))
			return false;
		return true;
	}

}
