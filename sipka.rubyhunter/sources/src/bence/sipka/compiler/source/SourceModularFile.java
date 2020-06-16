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
import java.io.OutputStream;
import java.io.UncheckedIOException;

import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.HashContentDescriptor;

public class SourceModularFile extends SakerFileBase {
	protected final SourceWritable source;
	private String headerComment = null;
	private ContentDescriptor contentDescriptor;

	public SourceModularFile(String name, SourceWritable source) {
		super(name);
		if (name.endsWith(".c") || name.endsWith(".cpp") || name.endsWith(".java")) {
			headerComment = "/* Auto-generated file, do not modify */\n";
		}
		this.source = source;
	}

	public SourceModularFile setHeaderComment(String headerComment) {
		this.headerComment = headerComment;
		return this;
	}

	@Override
	public void writeToStreamImpl(OutputStream os) throws IOException {
		if (headerComment != null) {
			os.write(headerComment.getBytes());
		}
		source.write(os);
	}

	@Override
	public ContentDescriptor getContentDescriptor() {
		if (contentDescriptor == null) {
			try {
				return HashContentDescriptor.hash(getBytesImpl());
			} catch (IOException e) {
				throw new UncheckedIOException(e);
			}
		}
		return contentDescriptor;
	}

	public void setContentDescriptor(ContentDescriptor contentDescriptor) {
		this.contentDescriptor = contentDescriptor;
	}
}
