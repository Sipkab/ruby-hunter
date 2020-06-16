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
package bence.sipka.compiler.source;

import java.util.Map;

import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.SerializableContentDescriptor;

public class TemplatedSourceModularFile extends SourceModularFile {

	public TemplatedSourceModularFile(String name, TemplatedSource source) {
		super(name, source);
	}

	public TemplatedSourceModularFile setThis(Object dependency) {
		((TemplatedSource) source).setThis(dependency);
		return this;
	}

	public TemplatedSourceModularFile setValueMap(Map<String, Object> valuemap) {
		((TemplatedSource) source).setValueMap(valuemap);
		return this;
	}

	@Override
	public ContentDescriptor getContentDescriptor() {
		return new SerializableContentDescriptor(((TemplatedSource) source).getValueMap());
	}
}
