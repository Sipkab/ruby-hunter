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
package bence.sipka.compiler.types.flags;

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

public class FlagsParser {
	private static final String NO_FLAG_ENTRY_NAME = "NO_FLAG";

	private static final String ATTR_DECLARE_FLAG_NAME = "name";
	private static final String ATTR_FLAG_NAME = "name";
	private static final String ATTR_FLAG_VALUE = "value";

	private static final String NODE_FLAG_ENTRY = "flag";
	private static final String NODE_DECLARE_FLAG = "declare-flag";

	private static final String ATTR_BACKING_TYPE_NAME = "backing-type";

	private Map<String, TypeDeclaration> typeDeclarations;
	private List<Document> xmldocuments;

	boolean putNoFlag = true;

	private static int parseFlagValue(String value) {
		String[] split = value.split("[ |]+");
		int result = 0;
		for (int i = 0; i < split.length; i++) {
			String val = split[i];
			if (val.startsWith("0x")) {
				result |= Integer.parseUnsignedInt(val.substring(2), 16);
			} else if (val.startsWith("0b")) {
				result |= Integer.parseUnsignedInt(val.substring(2), 2);
			} else {
				result |= Integer.parseInt(val);
			}
		}
		return result;
	}

	public FlagsParser(Map<String, TypeDeclaration> typeDeclarations, List<Document> xmldocuments) {
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

		if (!n.getLocalName().equals(NODE_DECLARE_FLAG)) {
			return;
		}

		NamedNodeMap attrs = n.getAttributes();
		Node nameattr = attrs.getNamedItem(ATTR_DECLARE_FLAG_NAME);
		Node backingattr = attrs.getNamedItem(ATTR_BACKING_TYPE_NAME);
		if (nameattr == null)
			throw new RuntimeException("name attribute for flag declaration is missing");

		String name = nameattr.getNodeValue();
		if (typeDeclarations.containsKey(name))
			throw new RuntimeException("Flag already defined " + name);

		NavigableMap<String, Integer> values = new TreeMap<>();

		addFlagEntries(values, n.getChildNodes());

		if (putNoFlag && !values.containsKey(NO_FLAG_ENTRY_NAME)) {
			values.put(NO_FLAG_ENTRY_NAME, 0);
		}

		FlagType flag = new FlagType(name, values);
		if (backingattr != null) {
			flag.setBackingType(backingattr.getNodeValue());
		}
		typeDeclarations.put(name, flag);

	}

	private void addFlagEntries(Map<String, Integer> values, NodeList childNodes) {
		for (int i = 0; i < childNodes.getLength(); i++) {
			Node child = childNodes.item(i);
			if (child.getNodeType() != Node.ELEMENT_NODE)
				continue;

			String nodename = child.getLocalName();
			if (!NODE_FLAG_ENTRY.equals(nodename))
				throw new RuntimeException("Invalid child node name: " + nodename + " for flag declaration");

			NamedNodeMap attrs = child.getAttributes();
			Node nameattr = attrs.getNamedItem(ATTR_FLAG_NAME);
			Node valueattr = attrs.getNamedItem(ATTR_FLAG_VALUE);

			if (nameattr == null)
				throw new RuntimeException("name attribute for flag entry declaration is missing");
			if (valueattr == null)
				throw new RuntimeException("value attribute for flag entry declaration is missing");

			String val = valueattr.getNodeValue();
			final int flagvalue = parseFlagValue(val);

			values.put(nameattr.getNodeValue(), flagvalue);
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
