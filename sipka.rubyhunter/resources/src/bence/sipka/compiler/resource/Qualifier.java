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
package bence.sipka.compiler.resource;

import java.util.Collection;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;

import bence.sipka.compiler.types.TypeDeclaration;

public class Qualifier {
	private static final String ATTR_NAME = "name";
	private static final String ATTR_TYPE = "type";
	private static final String ATTR_DEFAULT = "defaultValue";

	private final String name;
	private final TypeDeclaration type;
	private final int priority;
	private final String defaultValue;

	public Qualifier(Node n, int priority, Collection<TypeDeclaration> xmltypes) {
		this.priority = priority;
		NamedNodeMap attrs = n.getAttributes();

		Node nameattr = attrs.getNamedItem(ATTR_NAME);
		Node typeattr = attrs.getNamedItem(ATTR_TYPE);
		Node defaultattr = attrs.getNamedItem(ATTR_DEFAULT);

		if (nameattr == null)
			throw new RuntimeException("name attribute for qualifier declaration is missing");
		if (typeattr == null)
			throw new RuntimeException("type attribute for qualifier declaration is missing");
		if (defaultattr == null)
			throw new RuntimeException("defaultValue attribute for qualifier declaration is missing");

		this.name = nameattr.getNodeValue();

		this.type = xmltypes.stream().filter(t -> t.getName().equals(typeattr.getNodeValue())).findAny().orElse(null);
		this.defaultValue = defaultattr.getNodeValue();
		if (this.type == null)
			throw new RuntimeException("Type declaration not found: " + typeattr.getNodeValue());
	}

	public String getName() {
		return name;
	}

	public TypeDeclaration getTypeDeclaration() {
		return type;
	}

	public int getPriority() {
		return priority;
	}

	public String getDefaultValue() {
		return defaultValue;
	}

	@Override
	public String toString() {
		return "Qualifier [name=" + name + ", type=" + type + ", priority=" + priority + "]";
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
		Qualifier other = (Qualifier) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

}
