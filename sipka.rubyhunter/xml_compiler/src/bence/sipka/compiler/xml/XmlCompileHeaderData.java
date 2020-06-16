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
package bence.sipka.compiler.xml;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Arrays;
import java.util.List;

import bence.sipka.compiler.source.SourceTemplateTranslator;
import bence.sipka.compiler.types.TypeDeclaration;
import saker.build.thirdparty.saker.util.io.SerialUtils;

public class XmlCompileHeaderData implements Externalizable {
	private static final long serialVersionUID = 1L;

	List<TypeDeclaration> serializers;

	/**
	 * For {@link Externalizable}.
	 */
	public XmlCompileHeaderData() {
	}

	public String toCppName(String name) {
		if (Arrays.binarySearch(SourceTemplateTranslator.CPP_KEYWORDS, name) >= 0) {
			return name + "_";
		}
		return name;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		SerialUtils.writeExternalCollection(out, serializers);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		serializers = SerialUtils.readExternalImmutableList(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((serializers == null) ? 0 : serializers.hashCode());
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
		XmlCompileHeaderData other = (XmlCompileHeaderData) obj;
		if (serializers == null) {
			if (other.serializers != null)
				return false;
		} else if (!serializers.equals(other.serializers))
			return false;
		return true;
	}

}