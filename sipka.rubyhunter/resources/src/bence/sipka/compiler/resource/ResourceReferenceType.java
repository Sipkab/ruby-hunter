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
package bence.sipka.compiler.resource;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.util.NavigableMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.source.decl.LineSource;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.thirdparty.saker.util.io.SerialUtils;

public class ResourceReferenceType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;
	private static final String PREFIX = "@res/";

	//Don't rename this. it is referenced by source generators
	private NavigableMap<String, Integer> resourcesmap;

	/**
	 * For {@link Externalizable}.
	 */
	public ResourceReferenceType() {
	}

	public ResourceReferenceType(NavigableMap<String, Integer> resourcesmap) {
		super("resourcereference");
		this.resourcesmap = resourcesmap;
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		if (value.startsWith(PREFIX)) {
			String val = value.substring(PREFIX.length()).replace('\\', '/');
			Integer resid = resourcesmap.get(val);
			if (resid == null)
				throw new RuntimeException("Resource not found with name: " + value);
			IntegerType.INSTANCE.serialize(resid, os);
		} else {
			throw new RuntimeException("Invalid value specifier: \"" + value + "\" expected: " + PREFIX + " prefix");
		}
	}

	@Override
	public String getTypeRepresentation() {
		return "ResId";
	}

	@Override
	public String getStringValue(String value) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String toSourceForwardDeclaration() {
		try {
			return new LineSource("enum class ResId : uint32 { RES_INVALID = 0xFFFFFFFF /* -1 */ };").getAsString();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	public String toSourceString() {
		try {
			return new TemplatedSource(ResourceCompilerWorkerTaskFactory.descriptor::getInputStream,
					"resources_tostring.template.cpp").setThis(this).getAsString();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		super.writeExternal(out);
		SerialUtils.writeExternalMap(out, resourcesmap);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		super.readExternal(in);
		resourcesmap = SerialUtils.readExternalSortedImmutableNavigableMap(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + ((resourcesmap == null) ? 0 : resourcesmap.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (!super.equals(obj))
			return false;
		if (getClass() != obj.getClass())
			return false;
		ResourceReferenceType other = (ResourceReferenceType) obj;
		if (resourcesmap == null) {
			if (other.resourcesmap != null)
				return false;
		} else if (!resourcesmap.equals(other.resourcesmap))
			return false;
		return true;
	}

}
