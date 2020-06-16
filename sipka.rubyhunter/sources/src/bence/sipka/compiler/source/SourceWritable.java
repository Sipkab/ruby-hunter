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
package bence.sipka.compiler.source;

import java.io.IOException;
import java.io.OutputStream;

import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

public interface SourceWritable {
	public abstract void write(OutputStream out) throws IOException;

	public default byte[] getAsBytes() {
		try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream()) {
			try {
				write(baos);
			} catch (IOException e) {
				e.printStackTrace();
			}
			return baos.toByteArray();
		}
	}

	public default String getAsString() throws IOException {
		try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream()) {
			write(baos);
			return baos.toString();
		}
	}

}