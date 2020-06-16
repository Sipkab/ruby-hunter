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
package bence.sipka.compiler.shader;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import bence.sipka.compiler.shader.elements.UniformDeclaration;
import bence.sipka.compiler.shader.elements.VariableDeclaration;
import saker.build.file.SakerFile;
import saker.build.file.content.ContentDescriptor;
import sipka.syntax.parser.model.statement.Statement;

public class ShaderProgram implements Serializable {
	private static final long serialVersionUID = 1L;

	private ShaderResource vertexShader;
	private ShaderResource fragmentShader;

	private final String name;

	private ClassUrl classUrl;

	private ContentDescriptor definingFileContentDescriptor;

	public ShaderProgram(Statement stm, ShaderCollection shaders) {
		name = stm.getValue();
		Statement vertex = stm.firstScope("vertex_shader");
		Statement fragment = stm.firstScope("fragment_shader");
		if (vertex != null) {
			vertexShader = shaders.getShaderResourceForUri(name + ".vertex", ShaderResource.TYPE_VERTEX);
			vertexShader.define(shaders, vertex, this);
		}
		if (fragment != null) {
			fragmentShader = shaders.getShaderResourceForUri(name + ".fragment", ShaderResource.TYPE_FRAGMENT);
			fragmentShader.define(shaders, fragment, this);
		}
	}

	public void setClassUrl(ClassUrl classUrl) {
		this.classUrl = classUrl;
	}

	public ClassUrl getClassUrl() {
		return classUrl;
	}

	public boolean isCompleteProgram() {
		return vertexShader != null && fragmentShader != null && vertexShader.isDefined() && fragmentShader.isDefined();
	}

	public String getName() {
		return name;
	}

	public ShaderResource getVertexShader() {
		return vertexShader;
	}

	public ShaderResource getFragmentShader() {
		return fragmentShader;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((classUrl == null) ? 0 : classUrl.hashCode());
		result = prime * result + ((fragmentShader == null) ? 0 : fragmentShader.hashCode());
		result = prime * result + ((name == null) ? 0 : name.hashCode());
		result = prime * result + ((vertexShader == null) ? 0 : vertexShader.hashCode());
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
		ShaderProgram other = (ShaderProgram) obj;
		if (classUrl == null) {
			if (other.classUrl != null)
				return false;
		} else if (!classUrl.equals(other.classUrl))
			return false;
		if (fragmentShader == null) {
			if (other.fragmentShader != null)
				return false;
		} else if (!fragmentShader.equals(other.fragmentShader))
			return false;
		if (name == null) {
			if (other.name != null)
				return false;
		} else if (!name.equals(other.name))
			return false;
		if (vertexShader == null) {
			if (other.vertexShader != null)
				return false;
		} else if (!vertexShader.equals(other.vertexShader))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "ShaderProgram [" + (vertexShader != null ? "vertexShader=" + vertexShader + ", " : "")
				+ (fragmentShader != null ? "fragmentShader=" + fragmentShader + ", " : "")
				+ (name != null ? "name=" + name : "") + "]";
	}

	public void addShaderReferences() {
		if (isCompleteProgram()) {
			vertexShader.addReference();
			fragmentShader.addReference();
		}
	}

	public void validate() {
		Set<VariableDeclaration> vertexoutvars = new HashSet<>();
		vertexoutvars.addAll(vertexShader.getOutputVariables());
		outer:
		for (VariableDeclaration fragin : fragmentShader.getInputVariables()) {
			for (Iterator<VariableDeclaration> it = vertexoutvars.iterator(); it.hasNext();) {
				VariableDeclaration vertexout = it.next();
				if (fragin.typeAndNameEquals(vertexout)) {
					it.remove();
					continue outer;
				}
			}
			throw new RuntimeException("Failed to link program " + getName()
					+ ", not found vertex output variable for fragment input: " + fragin);
		}
		if (vertexoutvars.size() > 0) {
			throw new RuntimeException("Failed to link program " + getName()
					+ ", not found fragment input variable for vertex output variable(s):" + vertexoutvars);
		}

		List<UniformDeclaration> alluniform = getUniforms();

		Set<String> uniformnames = new HashSet<>();
		for (UniformDeclaration u : alluniform) {
			for (VariableDeclaration m : u.getMembers()) {
				String mname = m.getName();
				if (uniformnames.contains(mname)) {
					throw new RuntimeException("Uniform member name declared more than once in program: " + getName()
							+ " unifrom member: " + mname + " in struct: " + u.getName());
				}
				uniformnames.add(mname);
			}
		}
		uniformnames.clear();
		for (UniformDeclaration u : alluniform) {
			String mname = u.getName();
			if (uniformnames.contains(mname)) {
				throw new RuntimeException(
						"Uniform name declared more than once in program: " + getName() + " unifrom: " + mname);
			}
			uniformnames.add(mname);
		}
	}

	public Set<String> getUniformNames() {
		Set<String> uniformnames = new HashSet<>();
		for (UniformDeclaration u : vertexShader.getUniforms()) {
			for (VariableDeclaration m : u.getMembers()) {
				uniformnames.add(m.getName());
			}
		}
		for (UniformDeclaration u : fragmentShader.getUniforms()) {
			for (VariableDeclaration m : u.getMembers()) {
				uniformnames.add(m.getName());
			}
		}
		return uniformnames;
	}

	public List<UniformDeclaration> getUniforms() {
		List<UniformDeclaration> alluniform = new ArrayList<>();
		alluniform.addAll(vertexShader.getUniforms());
		alluniform.addAll(fragmentShader.getUniforms());
		return alluniform;
	}

	public void setFile(SakerFile f) {
		definingFileContentDescriptor = f.getContentDescriptor();
	}

	public ContentDescriptor getDefiningFileContentDescriptor() {
		return definingFileContentDescriptor;
	}

}
