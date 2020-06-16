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

public class ScopeDeclaration implements Serializable {
	private static final long serialVersionUID = 1L;

	private final TypeDeclaration sourceType;
	private final TypeDeclaration targetType;
	private String scoper;

	public ScopeDeclaration(TypeDeclaration sourceType, String scoper, TypeDeclaration targetType) {
		this.sourceType = sourceType;
		this.targetType = targetType;
		this.scoper = scoper;
	}

	public TypeDeclaration getSourceType() {
		return sourceType;
	}

	public TypeDeclaration getTargetType() {
		return targetType;
	}

	public String getScoper() {
		return scoper;
	}

	public void setScoper(String scoper) {
		this.scoper = scoper;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((scoper == null) ? 0 : scoper.hashCode());
		result = prime * result + ((sourceType == null) ? 0 : sourceType.hashCode());
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
		ScopeDeclaration other = (ScopeDeclaration) obj;
		if (scoper == null) {
			if (other.scoper != null)
				return false;
		} else if (!scoper.equals(other.scoper))
			return false;
		if (sourceType == null) {
			if (other.sourceType != null)
				return false;
		} else if (!sourceType.equals(other.sourceType))
			return false;
		return true;
	}

}
