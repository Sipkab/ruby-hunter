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
import java.io.Serializable;

import bence.sipka.compiler.types.builtin.FloatType;

public class Vector implements Serializable {
	private static final long serialVersionUID = 1L;

	public float x = 0.0f;
	public float y = 0.0f;
	public float z = 0.0f;
	public float w = 1.0f;

	public Vector() {
	}

	public Vector(String... values) {
		this(0, values);
	}

	public Vector(int index, String[] values) {
		if (values.length > index) {
			x = Float.parseFloat(values[index]);
			if (values.length > index + 1) {
				y = Float.parseFloat(values[index + 1]);
				if (values.length > index + 2) {
					z = Float.parseFloat(values[index + 2]);
					if (values.length > index + 3) {
						w = Float.parseFloat(values[index + 3]);
					}
				}
			}
		}
	}

	public Vector(float x, float y, float z, float w) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
	}

	public Vector(float x, float y, float z) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	public Vector(float x, float y) {
		this.x = x;
		this.y = y;
	}

	public Vector normalizeW() {
		if (w == 0.0f) {
			return this;
		}
		return new Vector(x / w, y / w, z / w, 1.0f);
	}

	public Vector normalizeLength() {
		double div = Math.sqrt(x * x + y * y + z * z);
		return new Vector((float) (x / div), (float) (y / div), (float) (z / div), 1.0f);
	}

	public void serializeXYZW(OutputStream os) throws IOException {
		FloatType.INSTANCE.serialize(x, os);
		FloatType.INSTANCE.serialize(y, os);
		FloatType.INSTANCE.serialize(z, os);
		FloatType.INSTANCE.serialize(w, os);
	}

	public void serializeXYZ(OutputStream os) throws IOException {
		FloatType.INSTANCE.serialize(x, os);
		FloatType.INSTANCE.serialize(y, os);
		FloatType.INSTANCE.serialize(z, os);
	}

	public void serializeXY(OutputStream os) throws IOException {
		FloatType.INSTANCE.serialize(x, os);
		FloatType.INSTANCE.serialize(y, os);
	}

	public static void serializeXY(OutputStream os, float x, float y) throws IOException {
		FloatType.INSTANCE.serialize(x, os);
		FloatType.INSTANCE.serialize(y, os);
	}

	@Override
	public String toString() {
		return "Vector [" + x + ", " + y + ", " + z + ", " + w + "]";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + Float.floatToIntBits(w);
		result = prime * result + Float.floatToIntBits(x);
		result = prime * result + Float.floatToIntBits(y);
		result = prime * result + Float.floatToIntBits(z);
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
		Vector other = (Vector) obj;
		if (Float.floatToIntBits(w) != Float.floatToIntBits(other.w))
			return false;
		if (Float.floatToIntBits(x) != Float.floatToIntBits(other.x))
			return false;
		if (Float.floatToIntBits(y) != Float.floatToIntBits(other.y))
			return false;
		if (Float.floatToIntBits(z) != Float.floatToIntBits(other.z))
			return false;
		return true;
	}

}