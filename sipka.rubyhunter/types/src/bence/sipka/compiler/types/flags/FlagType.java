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

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.util.NavigableMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.types.TypeDeclaration;
import bence.sipka.compiler.types.TypesWorkerTaskFactory;
import bence.sipka.compiler.types.builtin.IntegerType;
import saker.build.thirdparty.saker.util.io.SerialUtils;

public class FlagType extends TypeDeclaration {
	private static final long serialVersionUID = 1L;

	private NavigableMap<String, Integer> values;
	private String backingType = "uint32";

	/**
	 * For {@link Externalizable}.
	 */
	public FlagType() {
	}

	public FlagType(String name, NavigableMap<String, Integer> values) {
		super(name);
		this.values = values;
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		IntegerType.INSTANCE.serialize(getStringValue(value), os);
	}

	@Override
	public String toString() {
		return "FlagType: " + getName();
	}

	@Override
	public String getTypeRepresentation() {
		return getName();
	}

	@Override
	public String getStringValue(String value) {
		String[] flags = value.split("[| ]+");
		int result = 0;
		for (String f : flags) {
			Integer i = values.get(f);
			if (i == null)
				throw new RuntimeException("Flag not found: " + f + " in flag type: " + this);
			result |= i;
		}
		return "0x" + Integer.toHexString(result);
	}

	@Override
	public String toSourceForwardDeclaration() {
		return "enum class " + getName() + " : " + backingType + "; GENERATE_FLAG_OPERATORS(" + getName() + ", uint32)";
	}

	@Override
	public String toSourceDefinition() {
		try {
			return new TemplatedSource(TypesWorkerTaskFactory.descriptor::getInputStream, "flag.template.cpp").setThis(this)
					.getAsString();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	public String toSourceString() {
		try {
			return new TemplatedSource(TypesWorkerTaskFactory.descriptor::getInputStream, "flag_tostring.template.cpp")
					.setThis(this).getAsString();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	public String getBackingType() {
		return backingType;
	}

	public void setBackingType(String backingType) {
		this.backingType = backingType;
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		super.writeExternal(out);
		SerialUtils.writeExternalMap(out, values);
		out.writeObject(backingType);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		super.readExternal(in);
		values = SerialUtils.readExternalSortedImmutableNavigableMap(in);
		backingType = SerialUtils.readExternalObject(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = super.hashCode();
		result = prime * result + ((backingType == null) ? 0 : backingType.hashCode());
		result = prime * result + ((values == null) ? 0 : values.hashCode());
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
		FlagType other = (FlagType) obj;
		if (backingType == null) {
			if (other.backingType != null)
				return false;
		} else if (!backingType.equals(other.backingType))
			return false;
		if (values == null) {
			if (other.values != null)
				return false;
		} else if (!values.equals(other.values))
			return false;
		return true;
	}

}
