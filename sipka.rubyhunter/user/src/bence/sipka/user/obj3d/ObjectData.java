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
import java.util.List;

import saker.build.file.path.SakerPath;

public class ObjectData implements Serializable {
	private static final long serialVersionUID = 1L;

	SakerPath path;
	final String fileName;
	final int id;
	Vector origin = new Vector(0, 0, 0, 1);

	List<Vector> vertices = new ArrayList<>();
	List<Vector> textcoords = new ArrayList<>();
	List<Vector> normals = new ArrayList<>();
	List<Face> faces = new ArrayList<>();

	public ObjectData(int id, SakerPath path) {
		this.path = path;
		this.fileName = path.getFileName();
		this.id = id;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((faces == null) ? 0 : faces.hashCode());
		result = prime * result + id;
		result = prime * result + ((normals == null) ? 0 : normals.hashCode());
		result = prime * result + ((origin == null) ? 0 : origin.hashCode());
		result = prime * result + ((textcoords == null) ? 0 : textcoords.hashCode());
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
		ObjectData other = (ObjectData) obj;
		if (faces == null) {
			if (other.faces != null)
				return false;
		} else if (!faces.equals(other.faces))
			return false;
		if (id != other.id)
			return false;
		if (normals == null) {
			if (other.normals != null)
				return false;
		} else if (!normals.equals(other.normals))
			return false;
		if (origin == null) {
			if (other.origin != null)
				return false;
		} else if (!origin.equals(other.origin))
			return false;
		if (textcoords == null) {
			if (other.textcoords != null)
				return false;
		} else if (!textcoords.equals(other.textcoords))
			return false;
		if (vertices == null) {
			if (other.vertices != null)
				return false;
		} else if (!vertices.equals(other.vertices))
			return false;
		return true;
	}

}