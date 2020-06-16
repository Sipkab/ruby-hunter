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
package bence.sipka.compiler.types.enums;

import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.Objects;
import java.util.TreeMap;

import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.TypesTaskFactory;

public class EnumerationsParser {
	private static final String ATTR_ENUM_VALUE = "value";
	private static final String ATTR_DECLARE_ENUM_NAME = "name";
	private static final String ATTR_ENUM_NAME = "name";

	private static final String NODE_ENUM_ENTRY = "enum";
	private static final String NODE_DECLARE_ENUM = "declare-enum";

	private static final String ATTR_BACKING_TYPE_NAME = "backing-type";

	private Map<String, TypeDeclaration> typeDeclarations;
	private List<Document> xmldocuments;

	public EnumerationsParser(Map<String, TypeDeclaration> typeDeclarations, List<Document> xmldocuments) {
		this.typeDeclarations = typeDeclarations;
		this.xmldocuments = xmldocuments;
	}

	public void parse() {
		for (Document doc : xmldocuments) {
			Node root = doc.getFirstChild();
			String rooturi = root.getNamespaceURI();
			if (Objects.equals(TypesTaskFactory.CONFIG_NAMESPACE_URI, rooturi)) {
				parseNamespaceFile(doc);
			}
		}
	}

	private void parseChild(Node n) {
		if (n.getNodeType() != Node.ELEMENT_NODE)
			return;

		if (!n.getLocalName().equals(NODE_DECLARE_ENUM)) {
			return;
		}

		NamedNodeMap attrs = n.getAttributes();
		Node nameattr = attrs.getNamedItem(ATTR_DECLARE_ENUM_NAME);
		Node backingattr = attrs.getNamedItem(ATTR_BACKING_TYPE_NAME);
		if (nameattr == null)
			throw new RuntimeException("name attribute for enumeration declaration is missing");

		String name = nameattr.getNodeValue();
		if (typeDeclarations.containsKey(name))
			throw new RuntimeException("Enumeration already defined " + name);

		NavigableMap<String, Integer> values = new TreeMap<>();

		addEnumerationEntries(values, n.getChildNodes(), 0);

		EnumType enumtype = new EnumType(name, values);
		if (backingattr != null) {
			enumtype.setBackingType(backingattr.getNodeValue());
		}
		typeDeclarations.put(name, enumtype);
	}

	private void addEnumerationEntries(Map<String, Integer> values, NodeList childNodes, int id) {
		for (int i = 0; i < childNodes.getLength(); i++) {
			Node child = childNodes.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE)
				continue;

			String nodename = child.getLocalName();
			if (!NODE_ENUM_ENTRY.equals(nodename))
				throw new RuntimeException("Invalid child node name: " + nodename + " for enum declaration");

			NamedNodeMap attrs = child.getAttributes();
			Node nameattr = attrs.getNamedItem(ATTR_ENUM_NAME);
			Node valueattr = attrs.getNamedItem(ATTR_ENUM_VALUE);

			if (nameattr == null)
				throw new RuntimeException("name attribute for enumeration entry declaration is missing");

			final int enumvalue;
			if (valueattr == null) {
				while (values.containsValue(id)) {
					++id;
				}
				enumvalue = id++;
			} else {
				String val = valueattr.getNodeValue();
				if (val.startsWith("0x")) {
					enumvalue = Integer.parseUnsignedInt(val.substring(2), 16);
				} else if (val.startsWith("0b")) {
					enumvalue = Integer.parseUnsignedInt(val.substring(2), 2);
				} else {
					enumvalue = Integer.parseInt(val);
				}
			}
			values.put(nameattr.getNodeValue(), enumvalue);
		}
	}

	private void parseNamespaceFile(Document doc) {
		Node rootnode = doc.getChildNodes().item(0);
		NodeList children = rootnode.getChildNodes();
		int length = children.getLength();
		for (int i = 0; i < length; i++) {
			parseChild(children.item(i));
		}
	}
}
