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
package bence.sipka.compiler.source.precompile;

import java.io.IOException;
import java.io.OutputStream;

import bence.sipka.compiler.source.SourceWritable;

public class IncludeDecl implements SourceWritable {
	private final String path;

	public IncludeDecl(String path0, String... path) {
		String srcval = path0;
		for (int i = 0; i < path.length; i++) {
			srcval += "/" + path[i];
		}
		this.path = srcval;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(("#include <" + path + ">" + "\n").getBytes());
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((path == null) ? 0 : path.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		IncludeDecl other = (IncludeDecl) obj;
		if (path == null) {
			if (other.path != null)
				return false;
		} else if (!path.equals(other.path))
			return false;
		return true;
	}

}
