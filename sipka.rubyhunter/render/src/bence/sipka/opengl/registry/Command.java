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

public class Command implements Serializable {
	private static final long serialVersionUID = 1L;

	public static class Param implements Serializable {
		private static final long serialVersionUID = 1L;

		String type = "void";
		String name;
		String data = "";
		String group;

		public Param(Node node) {
			NamedNodeMap attrs = node.getAttributes();
			group = Registry.getAttrValue(attrs, "group");

			NodeList children = node.getChildNodes();
			int length = children.getLength();
			for (int i = 0; i < length; i++) {
				Node child = children.item(i);
				switch (child.getNodeType()) {
					case Node.ELEMENT_NODE: {
						switch (child.getNodeName()) {
							case "ptype": {
								type = child.getTextContent();
								data += type;
								break;
							}
							case "name": {
								name = child.getTextContent();
								data += name;
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
			if (type == null || name == null) {
				throw new RuntimeException("Param unspecified, type: " + type + " name: " + name);
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
			result = prime * result + ((data == null) ? 0 : data.hashCode());
			result = prime * result + ((group == null) ? 0 : group.hashCode());
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
			Param other = (Param) obj;
			if (data == null) {
				if (other.data != null)
					return false;
			} else if (!data.equals(other.data))
				return false;
			if (group == null) {
				if (other.group != null)
					return false;
			} else if (!group.equals(other.group))
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

	String name;
	String returnType = "void";
	String untilname = "";
	List<Command.Param> params = new ArrayList<>();

	public Command(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "proto": {
					NodeList protochildren = child.getChildNodes();
					int protolength = protochildren.getLength();
					for (int j = 0; j < protolength; j++) {
						Node protochild = protochildren.item(j);
						switch (protochild.getNodeType()) {
							case Node.ELEMENT_NODE: {
								switch (protochild.getNodeName()) {
									case "ptype": {
										returnType = protochild.getTextContent();
										untilname += returnType;
										break;
									}
									case "name": {
										name = protochild.getTextContent();
										break;
									}
									default: {
										throw new RuntimeException("Unknown child: " + protochild.getNodeName());
									}
								}
								break;
							}
							case Node.TEXT_NODE: {
								if (name == null) {
									untilname += protochild.getNodeValue();
								}
								break;
							}
							default: {
								break;
							}
						}
					}
					break;
				}
				case "param": {
					Command.Param p = new Param(child);
					params.add(p);
					break;
				}
				case "glx": {
					break;
				}
				case "alias": {
					break;
				}
				case "vecequiv": {
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	public String getParametersString() {
		return String.join(", ", (Iterable<String>) params.stream().map(p -> p.data)::iterator);
	}

	public String getParameterNamesString() {
		return String.join(", ", (Iterable<String>) params.stream().map(p -> p.name)::iterator);
	}

	@Override
	public String toString() {
		return returnType + " " + name + " (" + String.join(", ", params.stream().map(p -> p.data).toArray(String[]::new)) + ")";
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((params == null) ? 0 : params.hashCode());
		result = prime * result + ((returnType == null) ? 0 : returnType.hashCode());
		result = prime * result + ((untilname == null) ? 0 : untilname.hashCode());
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
		Command other = (Command) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (params == null) {
			if (other.params != null)
				return false;
		} else if (!params.equals(other.params))
			return false;
		if (returnType == null) {
			if (other.returnType != null)
				return false;
		} else if (!returnType.equals(other.returnType))
			return false;
		if (untilname == null) {
			if (other.untilname != null)
				return false;
		} else if (!untilname.equals(other.untilname))
			return false;
		return true;
	}

}