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

public class ConstNamespace extends SourceContainer {
	private final String name;

	public ConstNamespace() {
		this(null);
	}

	public ConstNamespace(String name) {
		this.name = name;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(("namespace " + (name == null ? "" : name) + " {").getBytes());
		super.write(out);

		out.write(("}" + (name == null ? "" : " // " + name)).getBytes());
	}

	public String getName() {
		return name;
	}

}
