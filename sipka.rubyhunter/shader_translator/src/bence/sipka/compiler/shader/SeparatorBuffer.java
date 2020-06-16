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
package bence.sipka.compiler.shader;

import java.util.List;

public class SeparatorBuffer {
	public interface Transformer<T> {
		public String transform(T obj);
	}

	private final String separator;
	private final StringBuilder builder = new StringBuilder();

	public SeparatorBuffer(String separator, Object... objects) {
		this.separator = separator;
		for (Object o : objects) {
			add(o);
		}
	}

	public <T> SeparatorBuffer(String separator, List<T> objects, Transformer<T> tr) {
		this.separator = separator;
		if (tr == null) {
			for (T o : objects) {
				add(o);
			}
		} else {
			for (T o : objects) {
				add(tr.transform(o));
			}
		}
	}

	public <T> SeparatorBuffer(String separator, List<T> objects) {
		this(separator, objects, null);
	}

	public <T> void add(T o) {
		add(null, o);
	}

	public <T> void add(Transformer<T> tr, T o) {
		if (o != null) {
			if (builder.length() != 0) {
				builder.append(separator);
			}
			builder.append(tr == null ? o : tr.transform(o));
		}
	}

	public void reset() {
		builder.setLength(0);
	}

	public int length() {
		return builder.length();
	}

	@Override
	public String toString() {
		return builder.toString();
	}
}
