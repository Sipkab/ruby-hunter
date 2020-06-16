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
import java.util.Map;

import bence.sipka.compiler.shader.translator2.program.elements.ShaderUniform;
import bence.sipka.compiler.shader.translator2.program.elements.ShaderVariable;
import sipka.syntax.parser.model.statement.modifiable.ModifiableStatement;

public class ShaderStage2 {
	private ShaderEnvironment environment;
	private ModifiableStatement statement;
	private List<ShaderUniform> uniforms = new ArrayList<>();
	private List<ShaderVariable> inputs = new ArrayList<>();
	private List<ShaderVariable> outputs = new ArrayList<>();

	public ShaderStage2(ShaderEnvironment env, ModifiableStatement stm) {
		this.environment = env;
		this.statement = stm;

		for (ModifiableStatement unistm; (unistm = stm.getChildNamed("uniform_var")) != null; stm.removeChild(unistm)) {
			String uniformname = unistm.getChildValue("uniform_name");
			ShaderUniform u = new ShaderUniform(uniformname);
			for (ModifiableStatement varstm : unistm.getChildrenNamed("variable")) {
				String vartypename = varstm.getChildValue("var_type");
				String varname = varstm.getChildValue("var_name");
				ShaderVariable var = new ShaderVariable(environment.getType(vartypename), varname);
				var.setAssignable(false);
				takeAttributes(varstm.getChildNamed("attributes"), var.getAttributes());

				u.getVariables().add(var);
			}
			uniforms.add(u);
		}

		for (ModifiableStatement varstm; (varstm = stm.getChildNamed("input_var")) != null; stm.removeChild(varstm)) {
			String vartypename = varstm.getChildValue("var_type");
			String varname = varstm.getChildValue("var_name");
			ShaderVariable var = new ShaderVariable(environment.getType(vartypename), varname);
			var.setAssignable(false);
			takeAttributes(varstm.getChildNamed("attributes"), var.getAttributes());

			inputs.add(var);
		}

		for (ModifiableStatement varstm; (varstm = stm.getChildNamed("output_var")) != null;) {
			String vartypename = varstm.getChildValue("var_type");
			String varname = varstm.getChildValue("var_name");

			ShaderVariable var = new ShaderVariable(environment.getType(vartypename), varname);
			takeAttributes(varstm.getChildNamed("attributes"), var.getAttributes());

			outputs.add(var);
			stm.removeChild(varstm);
			ModifiableStatement initialization = varstm.getChildNamed("initial_value");
			if (initialization != null) {
				ModifiableStatement initer = new ModifiableStatement("statement", "", null);
				initer.add(new ModifiableStatement("variable", varname, null));
				initer.add(new ModifiableStatement("operator", "=", null));
				initer.addChildren(initialization.getChildren());
			}
		}
	}

	public ModifiableStatement getStatement() {
		return statement;
	}

	public ShaderEnvironment getEnvironment() {
		return environment;
	}

	private static void takeAttributes(ModifiableStatement attrsstm, Map<String, String> attrs) {
		if (attrsstm == null) {
			return;
		}
		for (ModifiableStatement attr : attrsstm.getChildren()) {
			attrs.put(attr.getChildValue("key"), attr.getChildValue("value"));
		}
	}
}
