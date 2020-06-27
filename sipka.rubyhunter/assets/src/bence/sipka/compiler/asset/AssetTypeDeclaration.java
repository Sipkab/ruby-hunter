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
package bence.sipka.compiler.asset;

import java.io.Externalizable;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UncheckedIOException;
import java.util.NavigableMap;

import bence.sipka.compiler.source.TemplatedSource;
import bence.sipka.compiler.types.builtin.IntegerType;
import bence.sipka.compiler.types.enums.EnumType;

public class AssetTypeDeclaration extends EnumType {
	private static final long serialVersionUID = 1L;

	/**
	 * For {@link Externalizable}.
	 */
	public AssetTypeDeclaration() {
	}

	public AssetTypeDeclaration(NavigableMap<String, Integer> values) {
		super("RAssetFile", values);
	}

	@Override
	public void serialize(String value, OutputStream os) throws IOException {
		IntegerType.INSTANCE.serialize(getStringValue(value), os);
	}

	@Override
	public String toSourceForwardDeclaration() {
		return "enum class " + getName() + " : uint32 { INVALID_ASSET_IDENTIFIER = 0xFFFFFFFF /* -1 */ };";
	}

	@Override
	public String toSourceDefinition() {
		return null;
	}

	@Override
	public String toSourceString() {
		try {
			return new TemplatedSource(AssetsCompilerWorkerTaskFactory.descriptor::getInputStream,
					"assets_tostring.template.cpp").setThis(this).getAsString();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

}
