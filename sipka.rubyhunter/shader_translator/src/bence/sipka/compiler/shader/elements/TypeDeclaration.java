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

public class TypeDeclaration implements Serializable {
	private static final long serialVersionUID = 1L;

	private String name;
	private TypeDeclaration alias;
	private String frameworkType;

	public TypeDeclaration(String name, String frameworkType) {
		this.name = name;
		this.frameworkType = frameworkType;
	}

	public TypeDeclaration(String name, TypeDeclaration alias) {
		this.name = name;
		this.alias = alias;
	}

	public String getDeclaredName() {
		return alias == null ? name : alias.getDeclaredName();
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getDisplayName() {
		return name;
	}

	public boolean isAlias() {
		return alias != null;
	}

	public TypeDeclaration getRealType() {
		return alias == null ? this : alias.getRealType();
	}

	public String getFrameworkType() {
		return alias == null ? frameworkType : alias.getFrameworkType();
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((getDeclaredName() == null) ? 0 : getDeclaredName().hashCode());
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
		TypeDeclaration other = (TypeDeclaration) obj;
		if (getDeclaredName() == null) {
			if (other.getDeclaredName() != null)
				return false;
		} else if (!getDeclaredName().equals(other.getDeclaredName()))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "TypeDeclaration [" + (name != null ? "name=" + name + ", " : "") + (alias != null ? "alias=" + alias : "") + "]";
	}
}
