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
package bence.sipka.compiler.xml.declarations;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.TreeMap;

import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public final class ElementDeclaration implements Externalizable {
	private static final long serialVersionUID = 1L;

	private static final String ATTR_NAME = "name";
	private static final String ATTR_DEF_STATIC_TYPE = "defaultStaticType";

	public static final String NODE_DECL_ELEM = "declare-element";
	private static final String NODE_DECL_ATTR = "declare-attribute";
	private static final String NODE_INHERIT = "inherit";
	private static final String ATTR_ABSTRACT = "abstract";

	private String name;
	private String defStatic;

	private boolean elementAbstract;

	private NavigableMap<String, AttributeDeclaration> attributes = new TreeMap<>();
	private List<Inheritance> inherit = new ArrayList<>();

	/**
	 * For {@link Externalizable}.
	 */
	public ElementDeclaration() {
	}

	public ElementDeclaration(Node n) {
		NamedNodeMap attrs = n.getAttributes();

		Node nameattr = attrs.getNamedItem(ATTR_NAME);
		if (nameattr == null)
			throw new RuntimeException("name attribute is missing for element declaration");

		name = nameattr.getNodeValue();

		Node defstaticattr = attrs.getNamedItem(ATTR_DEF_STATIC_TYPE);
		defStatic = defstaticattr != null ? defstaticattr.getNodeValue() : null;

		Node abstractattr = attrs.getNamedItem(ATTR_ABSTRACT);
		elementAbstract = abstractattr != null && Boolean.parseBoolean(abstractattr.getNodeValue());

		NodeList children = n.getChildNodes();
		for (int i = 0; i < children.getLength(); i++) {
			Node child = children.item(i);
			if (child.getNodeType() != Element.ELEMENT_NODE)
				continue;

			switch (child.getLocalName()) {
				case NODE_DECL_ATTR: {
					AttributeDeclaration decl = new AttributeDeclaration(child);
					attributes.put(decl.getName(), decl);
					break;
				}
				case NODE_INHERIT: {
					inherit.add(new Inheritance(child));
					break;
				}
				default: {
					throw new RuntimeException("Invalid child for element declaration: " + child.getLocalName());
				}
			}
		}
	}

	public boolean isAbstract() {
		return elementAbstract;
	}

	public String getName() {
		return name;
	}

	public String getDefaultStaticType() {
		return defStatic;
	}

	public Collection<AttributeDeclaration> getAttributes() {
		return attributes.values();
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(name);
		out.writeObject(defStatic);
		out.writeBoolean(elementAbstract);
		SerialUtils.writeExternalMap(out, attributes);
		SerialUtils.writeExternalCollection(out, inherit);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		name = SerialUtils.readExternalObject(in);
		defStatic = SerialUtils.readExternalObject(in);
		elementAbstract = in.readBoolean();
		attributes = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		inherit = SerialUtils.readExternalImmutableList(in);
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
		ElementDeclaration other = (ElementDeclaration) obj;
		if (attributes == null) {
			if (other.attributes != null)
				return false;
		} else if (!attributes.equals(other.attributes))
			return false;
		if (defStatic == null) {
			if (other.defStatic != null)
				return false;
		} else if (!defStatic.equals(other.defStatic))
			return false;
		if (elementAbstract != other.elementAbstract)
			return false;
		if (inherit == null) {
			if (other.inherit != null)
				return false;
		} else if (!inherit.equals(other.inherit))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

	public AttributeDeclaration findAttribute(String localName, Map<String, ElementDeclaration> elements) {
		AttributeDeclaration attr = attributes.get(localName);
		if (attr != null)
			return attr;

		// search inheritances

		for (Inheritance inh : inherit) {
			ElementDeclaration base = elements.get(inh.getBase());
			AttributeDeclaration found = base.findAttribute(localName, elements);
			if (found != null)
				return found;
		}

		return null;
	}

	public void validate(Map<String, ElementDeclaration> elements) {
		if (defStatic != null) {
			ElementDeclaration found = elements.get(defStatic);
			if (!isInstanceOf(found, elements))
				throw new RuntimeException("Type: " + this.getName() + " does not inherit from: " + found.getName()
						+ " in default static type declaration");
		}

		for (Inheritance inh : inherit) {
			ElementDeclaration base = elements.get(inh.getBase());
			if (base == null)
				throw new RuntimeException("No base element declaration found for name: " + inh.getBase());
			for (AttributeDeclaration attr : attributes.values()) {
				AttributeDeclaration found = base.findAttribute(attr.getName(), elements);
				if (found != null) {
					if (!found.getType().equals(attr.getType())) {
						throw new RuntimeException("Attribute: " + attr.getName()
								+ " shadowed in inheritance hierarchy, with different types: " + attr.getType() + " "
								+ found.getType() + " for element: " + this.getName());
					}
				}
			}
		}
	}

	public boolean isInstanceOf(ElementDeclaration decl, Map<String, ElementDeclaration> elements) {
		if (decl.equals(this))
			return true;

		for (Inheritance inh : inherit) {
			String basename = inh.getBase();
			if (basename.equals(decl.getName()) || elements.get(basename).isInstanceOf(decl, elements))
				return true;
		}
		return false;
	}

}
