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

public class VarDeclarationStatement extends ShaderStatement {
	private static final long serialVersionUID = 1L;

	private VariableDeclaration declaration;
	private ExpressionStatement initialization;

	public VarDeclarationStatement(VariableDeclaration decl) {
		this.declaration = decl;
	}

	public VarDeclarationStatement(VariableDeclaration decl, ExpressionStatement initialization) {
		this.declaration = decl;
		this.initialization = initialization;
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putVariableDeclaration(this, os);
	}

	public VariableDeclaration getDeclaration() {
		return declaration;
	}

	public ExpressionStatement getInitialization() {
		return initialization;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((declaration == null) ? 0 : declaration.hashCode());
		result = prime * result + ((initialization == null) ? 0 : initialization.hashCode());
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
		VarDeclarationStatement other = (VarDeclarationStatement) obj;
		if (declaration == null) {
			if (other.declaration != null)
				return false;
		} else if (!declaration.equals(other.declaration))
			return false;
		if (initialization == null) {
			if (other.initialization != null)
				return false;
		} else if (!initialization.equals(other.initialization))
			return false;
		return true;
	}

}
