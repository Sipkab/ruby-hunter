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
package bence.sipka.compiler.shader.statement.control;

import java.io.PrintStream;

import bence.sipka.compiler.shader.statement.BlockStatement;
import bence.sipka.compiler.shader.statement.expression.ExpressionStatement;
import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class WhileControlStatement extends ControlStatement {
	private static final long serialVersionUID = 1L;

	private ExpressionStatement condition;
	private BlockStatement body;

	public WhileControlStatement(ExpressionStatement condition, BlockStatement body) {
		this.condition = condition;
		this.body = body;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putWhileControl(this, os);
	}

	public final ExpressionStatement getCondition() {
		return condition;
	}

	public final BlockStatement getBody() {
		return body;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((body == null) ? 0 : body.hashCode());
		result = prime * result + ((condition == null) ? 0 : condition.hashCode());
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
		WhileControlStatement other = (WhileControlStatement) obj;
		if (body == null) {
			if (other.body != null)
				return false;
		} else if (!body.equals(other.body))
			return false;
		if (condition == null) {
			if (other.condition != null)
				return false;
		} else if (!condition.equals(other.condition))
			return false;
		return true;
	}

}
