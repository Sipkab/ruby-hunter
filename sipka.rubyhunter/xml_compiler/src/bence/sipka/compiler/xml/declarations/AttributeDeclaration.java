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
package bence.sipka.compiler.xml.declarations;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public final class AttributeDeclaration implements Externalizable {
	private static final long serialVersionUID = 1L;

	private static final String ATTR_NAME = "name";
	private static final String ATTR_TYPE = "type";

	private String name;
	private String type;

	/**
	 * For {@link Externalizable}.
	 */
	public AttributeDeclaration() {
	}

	public AttributeDeclaration(Node n) {
		NamedNodeMap attrs = n.getAttributes();
		Node nameattr = attrs.getNamedItem(ATTR_NAME);
		Node typeattr = attrs.getNamedItem(ATTR_TYPE);

		if (nameattr == null)
			throw new RuntimeException("name attribute is missing for attribute declaration");
		if (typeattr == null)
			throw new RuntimeException("type attribute is missing for attribute declaration");

		this.name = nameattr.getNodeValue();
		this.type = typeattr.getNodeValue();
		if (type == null) {
			throw new RuntimeException("Type not founc for name: " + typeattr.getNodeValue());
		}
	}

	public String getName() {
		return name;
	}

	public String getType() {
		return type;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(name);
		out.writeObject(type);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		name = SerialUtils.readExternalObject(in);
		type = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((type == null) ? 0 : type.hashCode());
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
		AttributeDeclaration other = (AttributeDeclaration) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (type == null) {
			if (other.type != null)
				return false;
		} else if (!type.equals(other.type))
			return false;
		return true;
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder(getClass().getSimpleName());
		builder.append("[");
		if (name != null) {
			builder.append("name=");
			builder.append(name);
			builder.append(", ");
		}
		if (type != null) {
			builder.append("type=");
			builder.append(type);
		}
		builder.append("]");
		return builder.toString();
	}

}
