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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import bence.sipka.compiler.source.SourceTemplateTranslator.IncludeResolver;
import bence.sipka.compiler.source.SourceTemplateTranslator.TranslationHandler;
import saker.build.thirdparty.saker.util.io.function.IOSupplier;

public class TemplatedSource implements SourceWritable {

	private final IOSupplier<? extends InputStream> inputSupplier;
	private Map<String, Object> valueMap;
	private TranslationHandler handler;
	private IncludeResolver includeResolver;

	public TemplatedSource(IOSupplier<? extends InputStream> inputSupplier) {
		this.inputSupplier = inputSupplier;
	}

	public TemplatedSource(IncludeResolver includeresolver, String path) {
		this.includeResolver = includeresolver;
		this.inputSupplier = new IOSupplier<InputStream>() {
			@Override
			public InputStream get() throws IOException {
				return includeresolver.get(path);
			}

			@Override
			public String toString() {
				return path;
			}
		};
	}

	public TemplatedSource setValueMap(Map<String, Object> valmap) {
		this.valueMap = valmap;
		return this;
	}

	public TemplatedSource setHandler(TranslationHandler handler) {
		this.handler = handler;
		return this;
	}

	public TemplatedSource setIncludeResolver(IncludeResolver includeresolver) {
		this.includeResolver = includeresolver;
		return this;
	}

	public TemplatedSource setVar(String name, Object var) {
		if (valueMap == null) {
			valueMap = new HashMap<>();
		}
		valueMap.put(name, var);
		return this;
	}

	public TemplatedSource setThis(Object thiz) {
		return setVar("this", thiz);
	}

	@Override
	public void write(OutputStream out) throws IOException {
		try (InputStream is = inputSupplier.get()) {
			Objects.requireNonNull(is, "inputstream");
			SourceTemplateTranslator.translate(is, out, valueMap, handler, includeResolver);
		} catch (Exception e) {
			throw new IOException("Failed to translate: " + inputSupplier, e);
		}
	}

	public Map<String, Object> getValueMap() {
		return valueMap;
	}

}
