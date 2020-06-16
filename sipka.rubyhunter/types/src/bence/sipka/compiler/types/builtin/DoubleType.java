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

public final class DoubleType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	public static final DoubleType INSTANCE = new DoubleType();

	public DoubleType() {
		super("double");
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		try {
			final double val = Double.parseDouble(value);
			serialize(val, os);
		} catch (NumberFormatException e) {
			throw new IOException(e);
		}
	}

	public void serialize(double val, OutputStream os) throws IOException {
		LongType.INSTANCE.serialize(Double.doubleToRawLongBits(val), os);
	}

	@Override
	public String getTypeRepresentation() {
		return "double";
	}

	@Override
	public String getStringValue(String value) {
		return value;
	}

}
