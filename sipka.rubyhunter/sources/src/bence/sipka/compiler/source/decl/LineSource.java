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
package bence.sipka.compiler.source.decl;

import java.io.IOException;
import java.io.OutputStream;

import bence.sipka.compiler.source.SourceWritable;

public class LineSource implements SourceWritable {
	private final String line;

	public LineSource(String signature) {
		this.line = signature;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(line.getBytes());
	}

}