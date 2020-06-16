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
package bence.sipka.compiler.shader.translator2.program;

import java.util.ArrayList;
import java.util.List;

import sipka.syntax.parser.model.statement.modifiable.ModifiableStatement;

public class ShaderProgram2 {
	private final ShaderEnvironment environment;
	private List<ShaderStage2> stages = new ArrayList<>();
	private ModifiableStatement statement;

	public ShaderProgram2(ShaderEnvironment env, ModifiableStatement stm) {
		this.environment = env;
		this.statement = stm;

		ModifiableStatement vertex = stm.getChildNamed("vertex_shader");
		ModifiableStatement fragment = stm.getChildNamed("fragment_shader");

		if (vertex != null) {
			stages.add(new VertexStage(env, vertex));
		}
		if (fragment != null) {
			stages.add(new FragmentStage(env, vertex));
		}
	}

	public List<ShaderStage2> getStages() {
		return stages;
	}

	public ModifiableStatement getStatement() {
		return statement;
	}

	public ShaderEnvironment getEnvironment() {
		return environment;
	}
}
