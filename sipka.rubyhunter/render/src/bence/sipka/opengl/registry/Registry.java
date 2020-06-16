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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Registry implements Serializable {
	private static final long serialVersionUID = 1L;

	public static final String APIIMPORT_DEFINE = "GLGLUE_APIIMPORT";
	public static final String APIENTRY_DEFINE = "GLGLUE_APIENTRY";

	public static String getAttrValue(NamedNodeMap attrs, String name) {
		Node res = attrs.getNamedItem(name);
		return res == null ? null : res.getNodeValue();
	}

	public static String getAttrValue(NamedNodeMap attrs, String name, String defaultval) {
		Node res = attrs.getNamedItem(name);
		return res == null ? defaultval : res.getNodeValue();
	}

	public Map<String, Type> types = new HashMap<>();
	public List<Type> typesOrderList = new ArrayList<>();
	public Map<String, Group> groups = new HashMap<>();
	public Map<String, EnumValue> enumValues = new HashMap<>();
	public Map<String, Command> commands = new HashMap<>();
	public Map<String, Feature> features = new HashMap<>();
	public Map<String, Extension> extensions = new HashMap<>();

	public Registry(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "types": {
					parseTypes(child);
					break;
				}
				case "groups": {
					parseGroups(child);
					break;
				}
				case "enums": {
					parseEnums(child);
					break;
				}
				case "commands": {
					parseCommands(child);
					break;
				}
				case "feature": {
					Feature f = new Feature(child);
					if (features.containsKey(f.name)) {
						throw new RuntimeException("Feature redefined: " + f.name);
					}
					features.put(f.name, f);
					break;
				}
				case "extensions": {
					parseExtensions(child);
					break;
				}
				case "comment": {
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	private void parseExtensions(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "extension": {
					Extension e = new Extension(child);
					if (extensions.containsKey(e.name)) {
						throw new RuntimeException("Extension redefined: " + e.name);
					}
					extensions.put(e.name, e);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	private void parseCommands(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "command": {
					Command c = new Command(child);
					if (commands.containsKey(c.name)) {
						throw new RuntimeException("Command redefined: " + c.name);
					}
					commands.put(c.name, c);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	private void parseEnums(Node node) {
		NamedNodeMap attrs = node.getAttributes();
		Group g = new Group();
		g.name = Registry.getAttrValue(attrs, "group");
		g.comment = Registry.getAttrValue(attrs, "comment");

		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "enum": {
					EnumValue val = new EnumValue();
					NamedNodeMap childattrs = child.getAttributes();
					val.name = getAttrValue(childattrs, "name");
					val.value = getAttrValue(childattrs, "value");
					val.type = getAttrValue(childattrs, "type");
					val.api = getAttrValue(childattrs, "api", "");
					val.type = getAttrValue(childattrs, "type");
					val.alias = getAttrValue(childattrs, "alias");
					if (enumValues.containsKey(val.name + val.api)) {
						throw new RuntimeException("Enum redefined: " + val.name + val.api);
					}
					enumValues.put(val.name + val.api, val);
					g.enums.add(val.name);
					break;
				}
				case "unused": {
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
		if (g.name != null && !groups.containsKey(g.name)) {
			groups.put(g.name, g);
		}
	}

	private void parseGroups(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "group": {
					Group g = new Group(child);
					if (groups.containsKey(g.name)) {
						throw new RuntimeException("Duplicate group: " + g.name);
					}
					groups.put(g.name, g);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	private void parseTypes(Node node) {
		NodeList children = node.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE) {
				continue;
			}
			switch (child.getNodeName()) {
				case "type": {
					Type t = new Type(child);
					if (types.containsKey(t.name + t.api)) {
						throw new RuntimeException("Duplicate type: " + t.name + t.api);
					}
					types.put(t.name + t.api, t);
					typesOrderList.add(t);
					break;
				}
				default: {
					throw new RuntimeException("Unknown child: " + child.getNodeName());
				}
			}
		}
	}

	public Type getType(String name, String api) {
		if (name.equals("void"))
			return null;

		Type result = types.get(name + api);
		if (result != null)
			return result;
		result = types.get(name);
		if (result == null) {
			throw new RuntimeException("Result not found: " + name + " api: " + api);
		}
		return result;
	}

	public EnumValue getEnumValue(String name, String api) {
		EnumValue result = enumValues.get(name + api);
		if (result != null)
			return result;
		result = enumValues.get(name);
		if (result == null) {
			throw new RuntimeException("Result not found: " + name + " api: " + api);
		}
		return result;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((commands == null) ? 0 : commands.hashCode());
		result = prime * result + ((enumValues == null) ? 0 : enumValues.hashCode());
		result = prime * result + ((extensions == null) ? 0 : extensions.hashCode());
		result = prime * result + ((features == null) ? 0 : features.hashCode());
		result = prime * result + ((groups == null) ? 0 : groups.hashCode());
		result = prime * result + ((types == null) ? 0 : types.hashCode());
		result = prime * result + ((typesOrderList == null) ? 0 : typesOrderList.hashCode());
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
		Registry other = (Registry) obj;
		if (commands == null) {
			if (other.commands != null)
				return false;
		} else if (!commands.equals(other.commands))
			return false;
		if (enumValues == null) {
			if (other.enumValues != null)
				return false;
		} else if (!enumValues.equals(other.enumValues))
			return false;
		if (extensions == null) {
			if (other.extensions != null)
				return false;
		} else if (!extensions.equals(other.extensions))
			return false;
		if (features == null) {
			if (other.features != null)
				return false;
		} else if (!features.equals(other.features))
			return false;
		if (groups == null) {
			if (other.groups != null)
				return false;
		} else if (!groups.equals(other.groups))
			return false;
		if (types == null) {
			if (other.types != null)
				return false;
		} else if (!types.equals(other.types))
			return false;
		if (typesOrderList == null) {
			if (other.typesOrderList != null)
				return false;
		} else if (!typesOrderList.equals(other.typesOrderList))
			return false;
		return true;
	}

}
