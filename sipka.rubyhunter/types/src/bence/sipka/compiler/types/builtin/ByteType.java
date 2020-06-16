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

public final class ByteType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	public static final ByteType INSTANCE = new ByteType();

	public ByteType() {
		super("byte");
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		try {
			final int val;
			if (value.startsWith("0x")) {
				val = Integer.parseUnsignedInt(value.substring(2), 16);
				if (val > 0xFF) {
					throw new IOException("Short value is out of range: " + val);
				}
			} else if (value.startsWith("0b")) {
				val = Integer.parseUnsignedInt(value.substring(2), 2);
				if (val > 0xFF) {
					throw new IOException("Short value is out of range: " + val);
				}
			} else {
				val = Integer.parseInt(value);
				if (val > Byte.MAX_VALUE || val < Byte.MIN_VALUE) {
					throw new IOException("Short value is out of range: " + val);
				}
			}
			serialize((byte) val, os);
		} catch (NumberFormatException e) {
			throw new IOException(e);
		}
	}

	public void serialize(byte val, OutputStream os) throws IOException {
		os.write(val);
	}

	@Override
	public String getTypeRepresentation() {
		return "uint8";
	}

	@Override
	public String getStringValue(String value) {
		return value;
	}
}
