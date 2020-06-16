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
package bence.sipka.compiler.types;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public abstract class TypeDeclaration implements Comparable<TypeDeclaration>, Externalizable {
	private static final long serialVersionUID = 1L;

	private String name;

	/**
	 * For {@link Externalizable}.
	 */
	public TypeDeclaration() {
	}

	public TypeDeclaration(String name) {
		this.name = name;
	}

	/**
	 * @return The unique name representing this type
	 */
	public final String getName() {
		return name;
	}

	/**
	 * Serializes the input data in a manner, that it can be read easily from C++ runtime
	 * 
	 * @param value
	 *            The string value to parse
	 * @param os
	 *            Outputstream to write to
	 * @throws IOException
	 *             If the serialization fails
	 */
	public abstract void serialize(String value, OutputStream os) throws IOException;

	/**
	 * @return The C++ typename for this type
	 */
	public abstract String getTypeRepresentation();

	/**
	 * Convert input value to C++ source compilant string
	 * 
	 * @param value
	 *            the value to convert
	 * @return The C++ string
	 */
	public abstract String getStringValue(String value);

	@Override
	public String toString() {
		return "Type: " + name;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(name);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		name = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((name == null) ? 0 : name.hashCode());
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
		TypeDeclaration other = (TypeDeclaration) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

	public String toSourceString() {
		return null;
	}

	public String toSourceForwardDeclaration() {
		return null;
	}

	public String toSourceDefinition() {
		return null;
	}

	@Override
	public final int compareTo(TypeDeclaration o) {
		return name.compareTo(o.name);
	}

}
