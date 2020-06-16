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

public final class LongType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	public static final LongType INSTANCE = new LongType();

	private static final byte[] longToByteArray_BigEndian(long l) {
		byte[] result = new byte[8];

		result[0] = (byte) (l >>> 56);
		result[1] = (byte) (l >>> 48);
		result[2] = (byte) (l >>> 40);
		result[3] = (byte) (l >>> 32);
		result[4] = (byte) (l >>> 24);
		result[5] = (byte) (l >>> 16);
		result[6] = (byte) (l >>> 8);
		result[7] = (byte) (l /* >>> 0 */);

		return result;
	}

	public LongType() {
		super("long");
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		try {
			final long val;
			if (value.startsWith("0x")) {
				val = Long.parseUnsignedLong(value.substring(2), 16);
			} else if (value.startsWith("0b")) {
				val = Long.parseUnsignedLong(value.substring(2), 2);
			} else {
				val = Long.parseLong(value);
			}
			serialize(val, os);
		} catch (NumberFormatException e) {
			throw new IOException(e);
		}
	}

	public void serialize(long val, OutputStream os) throws IOException {
		os.write(longToByteArray_BigEndian(val));
	}

	@Override
	public String getTypeRepresentation() {
		return "uint64";
	}

	@Override
	public String getStringValue(String value) {
		return value;
	}

}
