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
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Extension implements Serializable {
	private static final long serialVersionUID = 1L;

	String name;
	Set<String> supported = new HashSet<>();
	String comment;
	List<RequireDefinition> requires = new ArrayList<>();
	List<RemoveDefinition> removes = new ArrayList<>();

	public Extension(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		name = Registry.getAttrValue(attrs, "name");
		for (String s : Registry.getAttrValue(attrs, "supported").split("[|]")) {
			supported.add(s);
		}
		comment = Registry.getAttrValue(attrs, "comment");

		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "require": {
					requires.add(new RequireDefinition(child));
					break;
				}
				case "remove": {
					removes.add(new RemoveDefinition(child));
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	public boolean supportsApi(String api) {
		return supported.contains(api);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((comment == null) ? 0 : comment.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((removes == null) ? 0 : removes.hashCode());
		result = prime * result + ((requires == null) ? 0 : requires.hashCode());
		result = prime * result + ((supported == null) ? 0 : supported.hashCode());
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
		Extension other = (Extension) obj;
		if (comment == null) {
			if (other.comment != null)
				return false;
		} else if (!comment.equals(other.comment))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (removes == null) {
			if (other.removes != null)
				return false;
		} else if (!removes.equals(other.removes))
			return false;
		if (requires == null) {
			if (other.requires != null)
				return false;
		} else if (!requires.equals(other.requires))
			return false;
		if (supported == null) {
			if (other.supported != null)
				return false;
		} else if (!supported.equals(other.supported))
			return false;
		return true;
	}

}