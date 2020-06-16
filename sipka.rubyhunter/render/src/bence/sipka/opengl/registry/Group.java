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
package bence.sipka.opengl.registry;

import java.io.Serializable;
import java.util.HashSet;
import java.util.Set;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Group implements Serializable {
	private static final long serialVersionUID = 1L;

	String name;
	String comment;
	Set<String> enums = new HashSet<>();

	public Group(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		name = Registry.getAttrValue(attrs, "name");

		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE)
				continue;
			switch (child.getNodeName()) {
				case "enum": {
					String enumname = child.getAttributes().getNamedItem("name").getNodeValue();
					if (enums.contains(enumname)) {
						throw new RuntimeException("Enum redefined " + enumname);
					}
					enums.add(enumname);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	public Group() {
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((comment == null) ? 0 : comment.hashCode());
		result = prime * result + ((enums == null) ? 0 : enums.hashCode());
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
		Group other = (Group) obj;
		if (comment == null) {
			if (other.comment != null)
				return false;
		} else if (!comment.equals(other.comment))
			return false;
		if (enums == null) {
			if (other.enums != null)
				return false;
		} else if (!enums.equals(other.enums))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

}