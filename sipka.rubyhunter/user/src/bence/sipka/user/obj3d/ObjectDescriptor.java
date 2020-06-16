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

public class ObjectDescriptor implements Serializable {
	private static final long serialVersionUID = 1L;

	static class TriangleRegion implements Serializable {
		private static final long serialVersionUID = 1L;

		public final Material material;
		public final int materialIndex;
		public final int vertexStartIndex;
		public final int vertexCount;

		public TriangleRegion(Material material, int materialIndex, int vertexStartIndex, int vertexCount) {
			this.material = material;
			this.materialIndex = materialIndex;
			this.vertexStartIndex = vertexStartIndex;
			this.vertexCount = vertexCount;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((material == null) ? 0 : material.hashCode());
			result = prime * result + materialIndex;
			result = prime * result + vertexCount;
			result = prime * result + vertexStartIndex;
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
			ObjectDescriptor.TriangleRegion other = (ObjectDescriptor.TriangleRegion) obj;
			if (material == null) {
				if (other.material != null)
					return false;
			} else if (!material.equals(other.material))
				return false;
			if (materialIndex != other.materialIndex)
				return false;
			if (vertexCount != other.vertexCount)
				return false;
			if (vertexStartIndex != other.vertexStartIndex)
				return false;
			return true;
		}

	}

	List<ObjectDescriptor.TriangleRegion> triangles = new ArrayList<>();
	int colored = 0;
	int textured = 0;

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + colored;
		result = prime * result + textured;
		result = prime * result + ((triangles == null) ? 0 : triangles.hashCode());
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
		ObjectDescriptor other = (ObjectDescriptor) obj;
		if (colored != other.colored)
			return false;
		if (textured != other.textured)
			return false;
		if (triangles == null) {
			if (other.triangles != null)
				return false;
		} else if (!triangles.equals(other.triangles))
			return false;
		return true;
	}

}