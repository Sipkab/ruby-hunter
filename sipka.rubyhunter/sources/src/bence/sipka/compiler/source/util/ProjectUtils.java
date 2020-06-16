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
package bence.sipka.compiler.source.util;

import java.util.Collection;
import java.util.function.Consumer;

import bence.sipka.utils.BundleContentAccess;
import saker.build.file.SakerDirectory;

public class ProjectUtils {

	private ProjectUtils() {
		throw new UnsupportedOperationException();
	}

//	public static void copySources(final String dir, ModularDirectory outdir, Collection<BundleEntryFile> files, Consumer<ModularFile> consumer) {
//		// use prevdir variable to store the previous recursive copy call.
//		// else the "a" directory will be copied when the files "a/b.txt" and "a/c.txt" occur
//		String prevdir = null;
//		for (BundleEntryFile e : (Iterable<BundleEntryFile>) files.stream().filter(e -> e.getPath().startsWith(dir)).sorted()::iterator) {
//			String entrypath = e.getPath();
//			int slashindex = entrypath.indexOf('/', dir.length());
//			if (slashindex >= 0) {
//				ModularDirectory pdir = outdir.getDirectoryCreate(entrypath.substring(dir.length(), slashindex));
//				String ndir = entrypath.substring(0, slashindex + 1);
//				if (ndir.equals(prevdir)) {
//					continue;
//				}
//				prevdir = ndir;
//				copySources(ndir, pdir, files, consumer);
//			} else {
//				ModularFile file = new BundleEntryModularFile(entrypath.substring(dir.length()), e);
//				// new InputStreamModularFile(entrypath.substring(dir.length()), e::open, e.getLastModified());
//				outdir.add(file);
//				consumer.accept(file);
//			}
//		}
//	}
//
//	public static void copySources(final String dir, ModularDirectory outdir, Collection<BundleEntryFile> files) {
//		copySources(dir, outdir, files, f -> {
//		});
//	}
}
