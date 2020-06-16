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
package bence.sipka.compiler.shader.statement.expression;

import java.util.Collections;
import java.util.List;

import bence.sipka.compiler.shader.elements.ScopeDeclaration;
import bence.sipka.compiler.shader.elements.TypeDeclaration;
import bence.sipka.compiler.shader.statement.ShaderStatement;

public abstract class ExpressionStatement extends ShaderStatement {
	private static final long serialVersionUID = 1L;

	private TypeDeclaration type;
	private List<ScopeDeclaration> scopes;

	public ExpressionStatement(TypeDeclaration type) {
		this.type = type;
		this.scopes = Collections.emptyList();
	}

	public ExpressionStatement(TypeDeclaration type, List<ScopeDeclaration> scopes) {
		this.type = type;
		this.scopes = scopes;
	}

	public List<ScopeDeclaration> getScopes() {
		return scopes;
	}

	public final TypeDeclaration getExpressionType() {
		if (scopes.size() > 0) {
			return scopes.get(scopes.size() - 1).getTargetType();
		}
		return type;
	}
}
