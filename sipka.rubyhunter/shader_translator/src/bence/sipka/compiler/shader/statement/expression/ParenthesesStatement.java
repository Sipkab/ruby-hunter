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

import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class ParenthesesStatement extends ExpressionStatement {
	private static final long serialVersionUID = 1L;

	private ExpressionStatement enclosed;

	public ParenthesesStatement(ExpressionStatement enclosed) {
		super(enclosed.getExpressionType(), enclosed.getScopes());
		this.enclosed = enclosed;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putParentheses(this, os);
	}

	public ExpressionStatement getEnclosedExpression() {
		return enclosed;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((enclosed == null) ? 0 : enclosed.hashCode());
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
		ParenthesesStatement other = (ParenthesesStatement) obj;
		if (enclosed == null) {
			if (other.enclosed != null)
				return false;
		} else if (!enclosed.equals(other.enclosed))
			return false;
		return true;
	}

}
