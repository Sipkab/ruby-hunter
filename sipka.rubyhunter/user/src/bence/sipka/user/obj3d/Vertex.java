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
package bence.sipka.user.obj3d;

import java.io.IOException;
import java.io.OutputStream;
import java.io.Serializable;

public class Vertex implements Serializable {
	private static final long serialVersionUID = 1L;

	public final Vector pos;
	public final Vector tex;
	public final Vector norm;

	public Vertex(VertexIndex index, ObjectData obj) {
		pos = obj.vertices.get(index.posIndex);
		tex = index.textureIndex >= 0 ? obj.textcoords.get(index.textureIndex) : null;
		norm = index.normalIndex >= 0 ? obj.normals.get(index.normalIndex) : null;
	}

	public void serialize(OutputStream os) throws IOException {
		pos.serializeXYZ(os);
		norm.normalizeLength().serializeXYZ(os);
		if (tex != null) {
			tex.normalizeW().serializeXY(os);
		}
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((norm == null) ? 0 : norm.hashCode());
		result = prime * result + ((pos == null) ? 0 : pos.hashCode());
		result = prime * result + ((tex == null) ? 0 : tex.hashCode());
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
		Vertex other = (Vertex) obj;
		if (norm == null) {
			if (other.norm != null)
				return false;
		} else if (!norm.equals(other.norm))
			return false;
		if (pos == null) {
			if (other.pos != null)
				return false;
		} else if (!pos.equals(other.pos))
			return false;
		if (tex == null) {
			if (other.tex != null)
				return false;
		} else if (!tex.equals(other.tex))
			return false;
		return true;
	}

}