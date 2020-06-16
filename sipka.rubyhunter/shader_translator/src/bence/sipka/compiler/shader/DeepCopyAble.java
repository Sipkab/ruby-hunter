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

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;

import saker.build.thirdparty.saker.util.io.UnsyncByteArrayInputStream;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

@Deprecated
interface DeepCopyAble extends Serializable {
	@SuppressWarnings("unchecked")
	public static <T extends DeepCopyAble> T deepCopy(T o) {
		try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream()) {
			try (ObjectOutputStream oos = new ObjectOutputStream(baos)) {
				oos.writeObject(o);
			}
			try (ObjectInputStream ois = new ObjectInputStream(
					new UnsyncByteArrayInputStream(baos.toByteArrayRegion()))) {
				return (T) ois.readObject();
			}
		} catch (IOException | ClassNotFoundException e) {
			throw new RuntimeException(e);
		}
	}

	@SuppressWarnings("unchecked")
	default public <T extends DeepCopyAble> T deepCopy() {
		return deepCopy((T) this);
	}
}
