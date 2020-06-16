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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import bence.sipka.compiler.source.decl.LineSource;
import bence.sipka.compiler.source.precompile.IncludeDecl;
import saker.build.thirdparty.saker.util.io.UnsyncByteArrayOutputStream;

public class SourceContainer implements SourceWritable, Iterable<SourceWritable> {
	protected final List<SourceWritable> children = new ArrayList<>();

	public SourceContainer(SourceWritable... sources) {
		if (sources != null) {
			for (SourceWritable write : sources) {
				this.children.add(write);
			}
		}
	}

	public SourceContainer add(SourceWritable... sources) {
		if (sources != null) {
			for (SourceWritable write : sources) {
				if (write != null) {
					this.children.add(write);
				}
			}
		}
		return this;
	}

	public SourceContainer add(String line) {
		add(new LineSource(line));
		return this;
	}

	@Override
	public void write(OutputStream out) throws IOException {
		for (SourceWritable write : children) {
			write.write(out);
		}
	}

	public void clear() {
		children.clear();
	}

	public void writeToFile(File f) throws IOException {
		try (FileOutputStream out = new FileOutputStream(f)) {
			write(out);
		}
	}

	public String getAndRemoveIncludesAsString() {
		try (UnsyncByteArrayOutputStream baos = new UnsyncByteArrayOutputStream()) {
			writeAndRemoveIncludes(baos);
			return baos.toString();
		} catch (IOException e) {
			e.printStackTrace();
			return "";
		}
	}

	public void writeAndRemoveIncludes(OutputStream out) throws IOException {
		for (Iterator<SourceWritable> it = children.iterator(); it.hasNext();) {
			SourceWritable src = it.next();
			if (src instanceof IncludeDecl) {
				src.write(out);
				it.remove();
			} else if (src instanceof SourceContainer) {
				((SourceContainer) src).writeAndRemoveIncludes(out);
			}
		}
	}

	public SourceWritable takeIncludesAsSourceWritable() {
		SourceContainer result = new SourceContainer();
		takeIncludesAsSourceWritable(result);
		return result;
	}

	private void takeIncludesAsSourceWritable(SourceContainer target) {
		for (Iterator<SourceWritable> it = children.iterator(); it.hasNext();) {
			SourceWritable src = it.next();
			if (src instanceof IncludeDecl) {
				target.add(src);
				it.remove();
			} else if (src instanceof SourceContainer) {
				((SourceContainer) src).takeIncludesAsSourceWritable(target);
			}
		}
	}

	@Override
	public Iterator<SourceWritable> iterator() {
		return children.iterator();
	}

}
