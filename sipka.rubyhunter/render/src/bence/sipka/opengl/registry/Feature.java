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
import java.util.List;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Feature implements Serializable {
	private static final long serialVersionUID = 1L;

	String api;
	String name;
	String number;
	String comment;
	List<RequireDefinition> requires = new ArrayList<>();
	List<RemoveDefinition> removes = new ArrayList<>();

	public Feature(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		name = Registry.getAttrValue(attrs, "name");
		api = Registry.getAttrValue(attrs, "api");
		comment = Registry.getAttrValue(attrs, "comment");
		number = Registry.getAttrValue(attrs, "number");

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
					RemoveDefinition rem = new RemoveDefinition(child);
					removes.add(rem);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	public boolean hasCommand(String cmd) {
		for (RequireDefinition req : requires) {
			if (req.commands.contains(cmd)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((api == null) ? 0 : api.hashCode());
		result = prime * result + ((comment == null) ? 0 : comment.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((number == null) ? 0 : number.hashCode());
		result = prime * result + ((removes == null) ? 0 : removes.hashCode());
		result = prime * result + ((requires == null) ? 0 : requires.hashCode());
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
		Feature other = (Feature) obj;
		if (api == null) {
			if (other.api != null)
				return false;
		} else if (!api.equals(other.api))
			return false;
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
		if (number == null) {
			if (other.number != null)
				return false;
		} else if (!number.equals(other.number))
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
		return true;
	}

}