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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Face implements Iterable<Vertex>, Serializable {
	private static final long serialVersionUID = 1L;

	transient ObjectData obj;
	int objectId;
	Material material;
	List<VertexIndex> vertices = new ArrayList<>();

	public Face(ObjectData obj) {
		this.obj = obj;
		this.objectId = obj.id;
	}

	public Material getMaterial() {
		return material;
	}

	@Override
	public String toString() {
		return "Face [vertices=" + vertices + "]";
	}

	@Override
	public Iterator<Vertex> iterator() {
		return vertices.stream().map(i -> new Vertex(i, obj)).iterator();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((material == null) ? 0 : material.hashCode());
		result = prime * result + objectId;
		result = prime * result + ((vertices == null) ? 0 : vertices.hashCode());
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
		Face other = (Face) obj;
		if (material == null) {
			if (other.material != null)
				return false;
		} else if (!material.equals(other.material))
			return false;
		if (objectId != other.objectId)
			return false;
		if (vertices == null) {
			if (other.vertices != null)
				return false;
		} else if (!vertices.equals(other.vertices))
			return false;
		return true;
	}

}