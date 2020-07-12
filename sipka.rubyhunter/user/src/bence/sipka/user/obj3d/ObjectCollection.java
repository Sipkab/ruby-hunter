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
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public class ObjectCollection implements Externalizable {
	private static final long serialVersionUID = 1L;

	public static class DuplicateObjectData implements Externalizable {
		private static final long serialVersionUID = 1L;

		String fileName;
		int objectDataId;
		Vector objectDataOrigin;
		MaterialLibrary materialLibrary;

		/**
		 * For {@link Externalizable}.
		 */
		public DuplicateObjectData() {
		}

		public DuplicateObjectData(String fileName, ObjectData objectData, MaterialLibrary materialLibrary) {
			this.fileName = fileName;
			this.objectDataId = objectData.id;
			this.objectDataOrigin = objectData.origin;
			this.materialLibrary = materialLibrary;
		}

		@Override
		public void writeExternal(ObjectOutput out) throws IOException {
			out.writeObject(fileName);
			out.writeInt(objectDataId);
			out.writeObject(objectDataOrigin);
			out.writeObject(materialLibrary);
		}

		@Override
		public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
			fileName = SerialUtils.readExternalObject(in);
			objectDataId = in.readInt();
			objectDataOrigin = SerialUtils.readExternalObject(in);
			materialLibrary = SerialUtils.readExternalObject(in);
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + ((fileName == null) ? 0 : fileName.hashCode());
			result = prime * result + ((materialLibrary == null) ? 0 : materialLibrary.hashCode());
			result = prime * result + objectDataId;
			result = prime * result + ((objectDataOrigin == null) ? 0 : objectDataOrigin.hashCode());
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
			if (objectDataId != other.objectDataId)
				return false;
			if (objectDataOrigin == null) {
				if (other.objectDataOrigin != null)
					return false;
			} else if (!objectDataOrigin.equals(other.objectDataOrigin))
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
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, objects);
		SerialUtils.writeExternalCollection(out, duplicateDatas);
		SerialUtils.writeExternalCollection(out, colorMaterials);
		SerialUtils.writeExternalCollection(out, textureMaterials);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		objects = SerialUtils.readExternalImmutableList(in);
		duplicateDatas = SerialUtils.readExternalImmutableList(in);
		colorMaterials = SerialUtils.readExternalImmutableList(in);
		textureMaterials = SerialUtils.readExternalImmutableList(in);
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