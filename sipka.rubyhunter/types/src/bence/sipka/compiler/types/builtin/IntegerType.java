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

public final class IntegerType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	public static final IntegerType INSTANCE = new IntegerType();

	private static final byte[] intToByteArray_BigEndian(int i) {
		byte[] result = new byte[4];

		result[0] = (byte) (i >>> 24);
		result[1] = (byte) (i >>> 16);
		result[2] = (byte) (i >>> 8);
		result[3] = (byte) (i /* >>> 0 */);

		return result;
	}

	public IntegerType() {
		super("int");
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		try {
			final int val;
			if (value.startsWith("0x")) {
				val = Integer.parseUnsignedInt(value.substring(2), 16);
			} else if (value.startsWith("0b")) {
				val = Integer.parseUnsignedInt(value.substring(2), 2);
			} else {
				val = Integer.parseInt(value);
			}
			serialize(val, os);
		} catch (NumberFormatException e) {
			throw new IOException(e);
		}
	}

	public void serialize(int val, OutputStream os) throws IOException {
		os.write(intToByteArray_BigEndian(val));
	}

	@Override
	public String getTypeRepresentation() {
		return "uint32";
	}

	@Override
	public String getStringValue(String value) {
		return value;
	}

}
