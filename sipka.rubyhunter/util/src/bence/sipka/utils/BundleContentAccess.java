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
package bence.sipka.utils;

import java.io.IOException;
import java.io.InputStream;
import java.util.NavigableSet;
import java.util.TreeSet;

import saker.build.thirdparty.saker.util.io.ByteArrayRegion;
import saker.build.thirdparty.saker.util.io.StreamUtils;
import saker.nest.bundle.NestBundleClassLoader;
import saker.nest.bundle.NestRepositoryBundle;

public class BundleContentAccess {
	private BundleContentAccess() {
		throw new UnsupportedOperationException();
	}

	public static BundleResourceSupplier getBundleResourceSupplier(String directory) {
		NestBundleClassLoader cl = (NestBundleClassLoader) BundleContentAccess.class.getClassLoader();
		NestRepositoryBundle bundle = cl.getBundle();
		if (directory == null) {
			return new BundleResourceSupplier() {
				@Override
				public InputStream getInputStream(String name) throws IOException {
					return bundle.openEntry(name);
				}

				@Override
				public NavigableSet<String> getEntries() {
					return bundle.getEntryNames();
				}
			};
		}
		String sdir;
		if (directory.endsWith("/")) {
			sdir = directory;
		} else {
			sdir = directory + "/";
		}
		return new BundleResourceSupplier() {
			@Override
			public InputStream getInputStream(String en) throws IOException {
				return bundle.openEntry(sdir + en);
			}

			@Override
			public NavigableSet<String> getEntries() {
				TreeSet<String> result = new TreeSet<>();
				int sdirlen = sdir.length();
				for (String en : bundle.getEntryNames()) {
					if (en.startsWith(sdir)) {
						result.add(en.substring(sdirlen));
					}
				}
				return result;
			}
		};
	}

	public interface BundleResourceSupplier {
		public InputStream getInputStream(String name) throws IOException;

		public default ByteArrayRegion getBytes(String name) throws IOException {
			try (InputStream is = getInputStream(name)) {
				return StreamUtils.readStreamFully(is);
			}
		}

		public NavigableSet<String> getEntries();

	}
}
