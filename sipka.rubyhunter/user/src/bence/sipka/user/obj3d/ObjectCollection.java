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

public class ObjectCollection implements Serializable {
	private static final long serialVersionUID = 1L;

	public static class DuplicateObjectData implements Serializable {
		private static final long serialVersionUID = 1L;

		String fileName;
		ObjectData objectData;
		MaterialLibrary materialLibrary;

		public DuplicateObjectData(String fileName, ObjectData objectData, MaterialLibrary materialLibrary) {
			this.fileName = fileName;
			this.objectData = objectData;
			this.materialLibrary = materialLibrary;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((fileName == null) ? 0 : fileName.hashCode());
			result = prime * result + ((materialLibrary == null) ? 0 : materialLibrary.hashCode());
			result = prime * result + ((objectData == null) ? 0 : objectData.hashCode());
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
			DuplicateObjectData other = (DuplicateObjectData) obj;
			if (fileName == null) {
				if (other.fileName != null)
					return false;
			} else if (!fileName.equals(other.fileName))
				return false;
			if (materialLibrary == null) {
				if (other.materialLibrary != null)
					return false;
			} else if (!materialLibrary.equals(other.materialLibrary))
				return false;
			if (objectData == null) {
				if (other.objectData != null)
					return false;
			} else if (!objectData.equals(other.objectData))
				return false;
			return true;
		}

	}

	List<ObjectData> objects = new ArrayList<>();
	List<DuplicateObjectData> duplicateDatas = new ArrayList<>();

	List<Material> colorMaterials = new ArrayList<>();
	List<Material> textureMaterials = new ArrayList<>();

	public ObjectCollection() {
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((colorMaterials == null) ? 0 : colorMaterials.hashCode());
		result = prime * result + ((duplicateDatas == null) ? 0 : duplicateDatas.hashCode());
		result = prime * result + ((objects == null) ? 0 : objects.hashCode());
		result = prime * result + ((textureMaterials == null) ? 0 : textureMaterials.hashCode());
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
		ObjectCollection other = (ObjectCollection) obj;
		if (colorMaterials == null) {
			if (other.colorMaterials != null)
				return false;
		} else if (!colorMaterials.equals(other.colorMaterials))
			return false;
		if (duplicateDatas == null) {
			if (other.duplicateDatas != null)
				return false;
		} else if (!duplicateDatas.equals(other.duplicateDatas))
			return false;
		if (objects == null) {
			if (other.objects != null)
				return false;
		} else if (!objects.equals(other.objects))
			return false;
		if (textureMaterials == null) {
			if (other.textureMaterials != null)
				return false;
		} else if (!textureMaterials.equals(other.textureMaterials))
			return false;
		return true;
	}

}