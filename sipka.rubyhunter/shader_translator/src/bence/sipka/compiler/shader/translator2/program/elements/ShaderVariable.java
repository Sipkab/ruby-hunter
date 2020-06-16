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
package bence.sipka.compiler.shader.translator2.program.elements;

import java.util.HashMap;
import java.util.Map;

public class ShaderVariable {

	public static final int FLAG_NON_ASSIGNABLE = 0x01;

	private ShaderType type;
	private String name;
	private Map<String, String> attributes = new HashMap<>();
	private int flags;

	public ShaderVariable(ShaderType type, String name) {
		this.type = type;
		this.name = name;
	}

	public final ShaderType getType() {
		return type;
	}

	public final void setType(ShaderType type) {
		this.type = type;
	}

	public final String getName() {
		return name;
	}

	public final void setName(String name) {
		this.name = name;
	}

	public Map<String, String> getAttributes() {
		return attributes;
	}

	public boolean isAssignable() {
		return ((flags & FLAG_NON_ASSIGNABLE) == FLAG_NON_ASSIGNABLE);
	}

	public void setAssignable(boolean assignable) {
		if (assignable) {
			flags = (FLAG_NON_ASSIGNABLE & ~flags);
		} else {
			flags = (FLAG_NON_ASSIGNABLE | flags);
		}
	}
}
