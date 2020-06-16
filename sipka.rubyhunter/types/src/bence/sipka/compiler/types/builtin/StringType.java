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
package bence.sipka.compiler.types.builtin;

import java.io.IOException;
import java.io.OutputStream;

import bence.sipka.compiler.types.TypeDeclaration;

public final class StringType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	public static final StringType INSTANCE = new StringType();

	public StringType() {
		super("string");
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		final int len = value.length();
		IntegerType.INSTANCE.serialize(len, os);
		os.write(value.getBytes());
	}

	@Override
	public String getTypeRepresentation() {
		return "FixedString";
	}

	@Override
	public String toSourceForwardDeclaration() {
		return "class " + getTypeRepresentation() + ";";
	}

	@Override
	public String getStringValue(String value) {
		return "\"" + value + "\"";
	}

}
