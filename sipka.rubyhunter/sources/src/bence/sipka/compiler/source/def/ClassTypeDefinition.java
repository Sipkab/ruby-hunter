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
package bence.sipka.compiler.source.def;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import bence.sipka.compiler.source.SourceWritable;
import bence.sipka.compiler.source.decl.MemberVisibility;
import bence.sipka.compiler.source.decl.TypeDecl;

public class ClassTypeDefinition implements SourceWritable {
	public static class TypeMember {
		private final MemberVisibility visibility;
		private final SourceWritable member;

		public TypeMember(MemberVisibility visibility, SourceWritable member) {
			this.visibility = visibility;
			this.member = member;
		}
	}

	private static final Comparator<TypeMember> VISIBILITY_COMPARATOR = new Comparator<TypeMember>() {
		@Override
		public int compare(TypeMember o1, TypeMember o2) {
			return o1.visibility.compareTo(o2.visibility);
		}
	};

	private final String typename;
	private final List<TypeMember> members = new ArrayList<TypeMember>();

	public ClassTypeDefinition(String typename, TypeMember... members) {
		this.typename = typename;
		if (members != null) {
			for (TypeMember mem : members) {
				this.members.add(mem);
			}
		}
	}

	public ClassTypeDefinition add(TypeMember... members) {
		if (members != null) {
			for (TypeMember mem : members) {
				this.members.add(mem);
			}
		}
		return this;
	}

	public ClassTypeDefinition addFriend(TypeDecl... types) {
		if (types != null) {
			for (TypeDecl t : types) {
				add(new TypeMember(MemberVisibility.PRIVATE, t.setFriend(true)));
			}
		}
		return this;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		out.write(("class " + typename + " {").getBytes());
		Collections.sort(members, VISIBILITY_COMPARATOR);
		int i = 0;
		for (MemberVisibility visibility : MemberVisibility.values()) {
			out.write((visibility.getSourceValue() + ":").getBytes());
			while (i < members.size() && members.get(i).visibility == visibility) {
				members.get(i).member.write(out);
				out.write('\n');
				i++;
			}
			out.write('\n');
		}
		out.write("};".getBytes());
		out.write('\n');
	}

}
