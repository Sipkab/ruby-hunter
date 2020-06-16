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
import java.util.ArrayList;
import java.util.List;

import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class BlockStatement extends ShaderStatement {
	private static final long serialVersionUID = 1L;

	private List<ShaderStatement> subStatements = new ArrayList<>();

	public void addSubStatement(ShaderStatement stm) {
		subStatements.add(stm);
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putBlockStatement(this, os);
	}

	public List<ShaderStatement> getSubStatements() {
		return subStatements;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((subStatements == null) ? 0 : subStatements.hashCode());
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
		BlockStatement other = (BlockStatement) obj;
		if (subStatements == null) {
			if (other.subStatements != null)
				return false;
		} else if (!subStatements.equals(other.subStatements))
			return false;
		return true;
	}

}
