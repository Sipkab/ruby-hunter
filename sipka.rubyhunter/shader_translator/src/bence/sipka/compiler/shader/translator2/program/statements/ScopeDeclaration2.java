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
package bence.sipka.compiler.shader.translator2.program.statements;

import bence.sipka.compiler.shader.translator2.program.elements.ShaderType;

public class ScopeDeclaration2 {
	private ShaderType source;
	private String scope;
	private ShaderType target;

	public ScopeDeclaration2(ShaderType source, String scope, ShaderType target) {
		this.source = source;
		this.scope = scope;
		this.target = target;
	}

	public final ShaderType getSource() {
		return source;
	}

	public final void setSource(ShaderType source) {
		this.source = source;
	}

	public final String getScope() {
		return scope;
	}

	public final void setScope(String scope) {
		this.scope = scope;
	}

	public final ShaderType getTarget() {
		return target;
	}

	public final void setTarget(ShaderType target) {
		this.target = target;
	}

}
