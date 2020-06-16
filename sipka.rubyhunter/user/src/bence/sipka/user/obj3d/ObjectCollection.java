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

	}

	List<ObjectData> objects = new ArrayList<>();
	List<DuplicateObjectData> duplicateDatas = new ArrayList<>();

	List<Material> colorMaterials = new ArrayList<>();
	List<Material> textureMaterials = new ArrayList<>();

	public ObjectCollection() {
	}

}