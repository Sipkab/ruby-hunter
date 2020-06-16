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
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class EnumerationClassDecl implements SourceWritable, Iterable<EnumerationClassDecl.EnumEntry> {
	public static class EnumEntry implements Serializable {
		private static final long serialVersionUID = 1L;

		public final String name;
		public final int value;

		public EnumEntry(String name, int value) {
			this.name = name;
			this.value = value;
		}

	}

	protected final String name;
	private List<EnumEntry> entries = new ArrayList<>();

	public EnumerationClassDecl(String name) {
		this.name = name;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(("enum class " + name + " : unsigned int {").getBytes());
		for (EnumEntry e : entries) {
			out.write((e.name + " =\t" + "0x" + Integer.toUnsignedString(e.value, 16) + ",\t/* " + e.value + " */").getBytes());
		}
		out.write(("_count_of_entries =\t" + entries.size()).getBytes());
		out.write("};".getBytes());
	}

	public EnumerationClassDecl addValue(String name, int value) {
		entries.add(new EnumEntry(name, value));
		return this;
	}

	public EnumerationClassDecl addValues(Map<String, Integer> values) {
		for (Entry<String, Integer> e : values.entrySet()) {
			addValue(e.getKey(), e.getValue());
		}
		return this;
	}

	public String getName() {
		return name;
	}

	@Override
	public Iterator<EnumEntry> iterator() {
		return entries.iterator();
	}

}
