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

import bence.sipka.compiler.shader.elements.TypeDeclaration;
import bence.sipka.compiler.shader.translator.ShaderTranslator;

public class IntegerLiteral extends Literal {
	private static final long serialVersionUID = 1L;

	public IntegerLiteral(String value, TypeDeclaration type) {
		super(value, type);
	}

	@Override
	public void write(ShaderTranslator translator, PrintStream os) {
		translator.putIntegerLiteral(this, os);
	}

}
