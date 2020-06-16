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

public class IfControlStatement extends ControlStatement {
	private static final long serialVersionUID = 1L;

	private ExpressionStatement condition;
	private BlockStatement ontrue;
	private BlockStatement onfalse;

	public IfControlStatement(ExpressionStatement condition, BlockStatement ontrue, BlockStatement onfalse) {
		this.condition = condition;
		this.ontrue = ontrue;
		this.onfalse = onfalse;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putIfControl(this, os);
	}

	public final ExpressionStatement getCondition() {
		return condition;
	}

	public final BlockStatement getOntrue() {
		return ontrue;
	}

	public final BlockStatement getOnfalse() {
		return onfalse;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((condition == null) ? 0 : condition.hashCode());
		result = prime * result + ((onfalse == null) ? 0 : onfalse.hashCode());
		result = prime * result + ((ontrue == null) ? 0 : ontrue.hashCode());
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
		IfControlStatement other = (IfControlStatement) obj;
		if (condition == null) {
			if (other.condition != null)
				return false;
		} else if (!condition.equals(other.condition))
			return false;
		if (onfalse == null) {
			if (other.onfalse != null)
				return false;
		} else if (!onfalse.equals(other.onfalse))
			return false;
		if (ontrue == null) {
			if (other.ontrue != null)
				return false;
		} else if (!ontrue.equals(other.ontrue))
			return false;
		return true;
	}

}
