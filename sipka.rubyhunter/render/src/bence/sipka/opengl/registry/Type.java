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

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Type implements Serializable {
	private static final long serialVersionUID = 1L;

	String name;
	String comment;
	String requires;
	String api;
	String data = "";

	public Type(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		requires = Registry.getAttrValue(attrs, "requires");
		name = Registry.getAttrValue(attrs, "name");
		api = Registry.getAttrValue(attrs, "api");
		if (api == null) {
			api = "";
		}
		comment = Registry.getAttrValue(attrs, "comment");

		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			switch (child.getNodeType()) {
				case Node.ELEMENT_NODE: {
					switch (child.getNodeName()) {
						case "name": {
							name = child.getTextContent();
							data += name;
							break;
						}
						case "apientry": {
							data += Registry.APIENTRY_DEFINE;
							break;
						}
						default: {
							throw new RuntimeException("Unknown child: " + child.getNodeName());
						}
					}
					break;
				}
				case Node.TEXT_NODE: {
					data += child.getNodeValue();
					break;
				}
				default: {
					break;
				}
			}
		}
	}

	@Override
	public String toString() {
		return data;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((api == null) ? 0 : api.hashCode());
		result = prime * result + ((comment == null) ? 0 : comment.hashCode());
		result = prime * result + ((data == null) ? 0 : data.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
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
		Type other = (Type) obj;
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
		if (data == null) {
			if (other.data != null)
				return false;
		} else if (!data.equals(other.data))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (requires == null) {
			if (other.requires != null)
				return false;
		} else if (!requires.equals(other.requires))
			return false;
		return true;
	}

}