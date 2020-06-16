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
package bence.sipka.compiler.source.decl;

import java.io.IOException;
import java.io.OutputStream;

import bence.sipka.compiler.source.SourceWritable;

public class TypeDecl implements SourceWritable {
	protected final String name;
	protected boolean isFriend;

	public TypeDecl(String name) {
		this.name = name;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(((isFriend ? "friend " : "") + "class " + name + ";").getBytes());
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
		TypeDecl other = (TypeDecl) obj;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "ClassDecl [" + name + "]";
	}

	public TypeDecl setFriend(boolean isFriend) {
		this.isFriend = isFriend;
		return this;
	}
}
