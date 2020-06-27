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

public class VertexIndex implements Externalizable {
	private static final long serialVersionUID = 1L;

	int posIndex = -1;
	int textureIndex = -1;
	int normalIndex = -1;

	public VertexIndex() {
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeInt(posIndex);
		out.writeInt(textureIndex);
		out.writeInt(normalIndex);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		posIndex = in.readInt();
		textureIndex = in.readInt();
		normalIndex = in.readInt();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + normalIndex;
		result = prime * result + posIndex;
		result = prime * result + textureIndex;
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
		VertexIndex other = (VertexIndex) obj;
		if (normalIndex != other.normalIndex)
			return false;
		if (posIndex != other.posIndex)
			return false;
		if (textureIndex != other.textureIndex)
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "VertexIndex [posIndex=" + posIndex + ", textureIndex=" + textureIndex + ", normalIndex=" + normalIndex
				+ "]";
	}

}