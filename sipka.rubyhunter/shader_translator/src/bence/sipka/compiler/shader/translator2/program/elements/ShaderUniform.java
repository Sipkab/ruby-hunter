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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ShaderUniform {
	private String name;
	private List<ShaderVariable> variables = new ArrayList<>();
	private Map<String, String> attributes = new HashMap<>();

	public ShaderUniform(String name) {
		this.name = name;
	}

	public final List<ShaderVariable> getVariables() {
		return variables;
	}

	public final String getName() {
		return name;
	}

	public Map<String, String> getAttributes() {
		return attributes;
	}
}
