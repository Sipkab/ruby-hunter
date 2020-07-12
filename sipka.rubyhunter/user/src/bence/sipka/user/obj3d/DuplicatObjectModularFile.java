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
import java.util.Arrays;

import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.exception.InvalidPathFormatException;
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.MultiContentDescriptor;
import saker.build.file.content.SerializableContentDescriptor;

public class DuplicatObjectModularFile extends SakerFileBase {
	private ObjectConfiguration coll;
	private int objectDataId;
	private Vector objectDataOrigin;
	private MaterialLibrary matlib;

	private ContentDescriptor contents;

	public DuplicatObjectModularFile(String name, ObjectConfiguration coll, int objectDataId, Vector objectDataOrigin,
			MaterialLibrary matlib) throws NullPointerException, InvalidPathFormatException {
		super(name);
		this.coll = coll;
		this.objectDataId = objectDataId;
		this.objectDataOrigin = objectDataOrigin;
		this.matlib = matlib;
		this.contents = MultiContentDescriptor.create(Arrays.asList(new SerializableContentDescriptor(coll),
				new SerializableContentDescriptor(objectDataId), new SerializableContentDescriptor(objectDataOrigin),
				new SerializableContentDescriptor(matlib)));
	}

	@Override
	public void writeToStreamImpl(OutputStream os) throws IOException {
		ObjectDescriptor desc = coll.getDescriptor(objectDataId);
		objectDataOrigin.normalizeW().serializeXYZ(os);
		IntegerType.INSTANCE.serialize(desc.colored, os);
		IntegerType.INSTANCE.serialize(desc.textured, os);
		for (ObjectDescriptor.TriangleRegion tri : desc.triangles) {
			int index;
			if (tri.material.hasTexture()) {
				Material newmat = matlib.get(coll.textureMaterials.get(tri.materialIndex).getName());
				index = coll.textureMaterials.indexOf(newmat);
			} else {
				Material newmat = matlib.get(coll.colorMaterials.get(tri.materialIndex).getName());
				index = coll.colorMaterials.indexOf(newmat);
			}
			IntegerType.INSTANCE.serialize(index, os);
			IntegerType.INSTANCE.serialize(tri.vertexStartIndex, os);
			IntegerType.INSTANCE.serialize(tri.vertexCount, os);
		}
	}

	@Override
	public ContentDescriptor getContentDescriptor() {
		return contents;
	}

}