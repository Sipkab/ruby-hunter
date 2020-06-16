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

public class ReqRemoveDef implements Serializable {
	private static final long serialVersionUID = 1L;

	String profile;
	String comment;
	String api;

	List<String> commands = new ArrayList<>();
	List<String> enums = new ArrayList<>();
	List<String> types = new ArrayList<>();

	public ReqRemoveDef(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		profile = Registry.getAttrValue(attrs, "profile");
		api = Registry.getAttrValue(attrs, "api");
		comment = Registry.getAttrValue(attrs, "comment");

		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "command": {
					commands.add(Registry.getAttrValue(child.getAttributes(), "name"));
					break;
				}
				case "enum": {
					enums.add(Registry.getAttrValue(child.getAttributes(), "name"));
					break;
				}
				case "type": {
					types.add(Registry.getAttrValue(child.getAttributes(), "name"));
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((api == null) ? 0 : api.hashCode());
		result = prime * result + ((commands == null) ? 0 : commands.hashCode());
		result = prime * result + ((comment == null) ? 0 : comment.hashCode());
		result = prime * result + ((enums == null) ? 0 : enums.hashCode());
		result = prime * result + ((profile == null) ? 0 : profile.hashCode());
		result = prime * result + ((types == null) ? 0 : types.hashCode());
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
		ReqRemoveDef other = (ReqRemoveDef) obj;
		if (api == null) {
			if (other.api != null)
				return false;
		} else if (!api.equals(other.api))
			return false;
		if (commands == null) {
			if (other.commands != null)
				return false;
		} else if (!commands.equals(other.commands))
			return false;
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
		if (profile == null) {
			if (other.profile != null)
				return false;
		} else if (!profile.equals(other.profile))
			return false;
		if (types == null) {
			if (other.types != null)
				return false;
		} else if (!types.equals(other.types))
			return false;
		return true;
	}

}