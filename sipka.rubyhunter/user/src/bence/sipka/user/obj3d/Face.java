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

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public class Face implements Externalizable {
	private static final long serialVersionUID = 1L;

	Material material;
	List<VertexIndex> vertices = new ArrayList<>();

	/**
	 * For {@link Externalizable}.
	 */
	public Face() {
	}

	public Face(Material material) {
		this.material = material;
	}

	public Material getMaterial() {
		return material;
	}

	@Override
	public String toString() {
		return "Face [vertices=" + vertices + "]";
	}

	public Iterator<Vertex> vertexIterator(ObjectData obj) {
		return vertices.stream().map(i -> new Vertex(i, obj)).iterator();
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(material);
		SerialUtils.writeExternalCollection(out, vertices);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		material = SerialUtils.readExternalObject(in);
		vertices = SerialUtils.readExternalImmutableList(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((material == null) ? 0 : material.hashCode());
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
		if (vertices == null) {
			if (other.vertices != null)
				return false;
		} else if (!vertices.equals(other.vertices))
			return false;
		return true;
	}

}