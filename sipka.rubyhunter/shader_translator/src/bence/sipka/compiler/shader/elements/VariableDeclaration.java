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
package bence.sipka.compiler.shader.elements;

import java.io.Serializable;
import java.util.Collections;
import java.util.List;

public class VariableDeclaration implements Serializable {
	private static final long serialVersionUID = 1L;

	private List<AttributeDeclaration> attributes = Collections.emptyList();

	private TypeDeclaration type;
	private String name;
	private boolean assignable = true;

	public VariableDeclaration(TypeDeclaration type, String name) {
		this.type = type;
		this.name = name;
	}

	public void setAttributes(List<AttributeDeclaration> attributes) {
		this.attributes = attributes;
	}

	public List<AttributeDeclaration> getAttributes() {
		return attributes;
	}

	public AttributeDeclaration getAttribute(String name) {
		for (AttributeDeclaration attr : attributes) {
			if (attr.getKey().equals(name)) {
				return attr;
			}
		}
		return null;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public TypeDeclaration getType() {
		return type;
	}

	public void setType(TypeDeclaration type) {
		this.type = type;
	}

	public boolean typeAndNameEquals(VariableDeclaration other) {
		return other.type.equals(type) && other.name.equals(name);
	}

	public boolean isAssignable() {
		return assignable;
	}

	public void setAssignable(boolean assignable) {
		this.assignable = assignable;
	}

	@Override
	public String toString() {
		return "VariableDeclaration [" + (type != null ? "type=" + type + ", " : "") + (name != null ? "name=" + name : "") + "]";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (assignable ? 1231 : 1237);
		result = prime * result + ((attributes == null) ? 0 : attributes.hashCode());
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
		VariableDeclaration other = (VariableDeclaration) obj;
		if (assignable != other.assignable)
			return false;
		if (attributes == null) {
			if (other.attributes != null)
				return false;
		} else if (!attributes.equals(other.attributes))
			return false;
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

}
