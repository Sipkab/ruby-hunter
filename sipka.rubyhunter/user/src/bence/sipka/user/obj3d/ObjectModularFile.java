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
import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.MultiContentDescriptor;
import saker.build.file.content.SerializableContentDescriptor;

public class ObjectModularFile extends SakerFileBase {
	private ObjectConfiguration coll;
	private ObjectData data;

	private ContentDescriptor contents;

	public ObjectModularFile(String filename, ObjectConfiguration coll, ObjectData data) {
		super(filename);
		this.coll = coll;
		this.data = data;

		contents = MultiContentDescriptor.create(
				Arrays.asList(new SerializableContentDescriptor(coll), new SerializableContentDescriptor(data)));
	}

	@Override
	public void writeToStreamImpl(OutputStream os) throws IOException {
		ObjectDescriptor desc = coll.getDescriptor(data.id);
		data.origin.normalizeW().serializeXYZ(os);
		IntegerType.INSTANCE.serialize(desc.colored, os);
		IntegerType.INSTANCE.serialize(desc.textured, os);
		for (ObjectDescriptor.TriangleRegion tri : desc.triangles) {
			IntegerType.INSTANCE.serialize(tri.materialIndex, os);
			IntegerType.INSTANCE.serialize(tri.vertexStartIndex, os);
			IntegerType.INSTANCE.serialize(tri.vertexCount, os);
		}
	}

	@Override
	public ContentDescriptor getContentDescriptor() {
		return contents;
	}

}