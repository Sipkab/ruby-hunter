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
package bence.sipka.compiler.shader.elements;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import bence.sipka.compiler.shader.ShaderResource;

public class UniformDeclaration implements Serializable {
	private static final long serialVersionUID = 1L;

	private final String name;
	private final List<VariableDeclaration> members = new ArrayList<>();
	private final ShaderResource parentShader;

	public UniformDeclaration(String name, ShaderResource parentShader) {
		this.name = name;
		this.parentShader = parentShader;
	}

	public void addMember(VariableDeclaration member) {
		members.add(member);
	}

	public String getName() {
		return name;
	}

	public List<VariableDeclaration> getMembers() {
		return members;
	}

	public ShaderResource getParentShader() {
		return parentShader;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((members == null) ? 0 : members.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((parentShader == null) ? 0 : parentShader.hashCode());
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
		UniformDeclaration other = (UniformDeclaration) obj;
		if (members == null) {
			if (other.members != null)
				return false;
		} else if (!members.equals(other.members))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		return true;
	}

}
