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

import org.w3c.dom.Node;

import saker.build.thirdparty.saker.util.io.SerialUtils;

public class Inheritance implements Externalizable {
	private static final long serialVersionUID = 1L;

	private static final String ATTR_BASE = "base";

	private String base;

	/**
	 * For {@link Externalizable}.
	 */
	public Inheritance() {
	}

	public Inheritance(Node n) {
		Node baseattr = n.getAttributes().getNamedItem(ATTR_BASE);
		if (baseattr == null)
			throw new RuntimeException("Inheritance base attribute is missing");

		base = baseattr.getNodeValue();
	}

	public String getBase() {
		return base;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeObject(base);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		base = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((base == null) ? 0 : base.hashCode());
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
		Inheritance other = (Inheritance) obj;
		if (base == null) {
			if (other.base != null)
				return false;
		} else if (!base.equals(other.base))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "Inheritance[" + (base != null ? "base=" + base : "") + "]";
	}

}
