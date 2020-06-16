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
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

public class MaterialLibrary implements Serializable {
	private static final long serialVersionUID = 1L;

	List<Material> materials = new ArrayList<>();

	public MaterialLibrary() {
	}

	public int indexof(String name) {
		for (int i = 0; i < materials.size(); i++) {
			if (materials.get(i).name.equals(name)) {
				return i;
			}
		}
		return -1;
	}

	public Material get(String name) {
		for (int i = 0; i < materials.size(); i++) {
			Material cur = materials.get(i);
			if (cur.name.equals(name)) {
				return cur;
			}
		}
		return null;
	}

	public Collection<Material> getColoreds() {
		return materials.stream().filter(m -> !m.hasTexture()).collect(Collectors.toList());
	}

	public Collection<Material> getTextureds() {
		return materials.stream().filter(Material::hasTexture).collect(Collectors.toList());
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((materials == null) ? 0 : materials.hashCode());
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
		MaterialLibrary other = (MaterialLibrary) obj;
		if (materials == null) {
			if (other.materials != null)
				return false;
		} else if (!materials.equals(other.materials))
			return false;
		return true;
	}

}