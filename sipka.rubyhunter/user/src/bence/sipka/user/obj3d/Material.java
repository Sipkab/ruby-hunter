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
package bence.sipka.user.obj3d;

import java.io.Serializable;

import saker.build.file.path.SakerPath;

public class Material implements Serializable {
	private static final long serialVersionUID = 1L;

	String name;
	SakerPath texture;
	Vector diffuseColor;
	Vector ambientColor;
	Vector specularColor;
	Float specularExponent;
	Float transparency;

	public Material(String name) {
		this.name = name;
	}

	public SakerPath getTexture() {
		return texture;
	}

	public Float getTransparency() {
		return transparency;
	}

	public boolean hasTexture() {
		return texture != null;
	}

	public String getName() {
		return name;
	}

	public Vector getDiffuseColor() {
		if (transparency == null) {
			return diffuseColor;
		}
		return new Vector(diffuseColor.x, diffuseColor.y, diffuseColor.z, diffuseColor.w * transparency);
	}

	public Vector getAmbientColor() {
		return ambientColor;
	}

	public Vector getSpecularColor() {
		return specularColor;
	}

	public Float getSpecularExponent() {
		return specularExponent;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((ambientColor == null) ? 0 : ambientColor.hashCode());
		result = prime * result + ((diffuseColor == null) ? 0 : diffuseColor.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((specularColor == null) ? 0 : specularColor.hashCode());
		result = prime * result + ((specularExponent == null) ? 0 : specularExponent.hashCode());
		result = prime * result + ((texture == null) ? 0 : texture.hashCode());
		result = prime * result + ((transparency == null) ? 0 : transparency.hashCode());
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
		Material other = (Material) obj;
		if (ambientColor == null) {
			if (other.ambientColor != null)
				return false;
		} else if (!ambientColor.equals(other.ambientColor))
			return false;
		if (diffuseColor == null) {
			if (other.diffuseColor != null)
				return false;
		} else if (!diffuseColor.equals(other.diffuseColor))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (specularColor == null) {
			if (other.specularColor != null)
				return false;
		} else if (!specularColor.equals(other.specularColor))
			return false;
		if (specularExponent == null) {
			if (other.specularExponent != null)
				return false;
		} else if (!specularExponent.equals(other.specularExponent))
			return false;
		if (texture == null) {
			if (other.texture != null)
				return false;
		} else if (!texture.equals(other.texture))
			return false;
		if (transparency == null) {
			if (other.transparency != null)
				return false;
		} else if (!transparency.equals(other.transparency))
			return false;
		return true;
	}

}