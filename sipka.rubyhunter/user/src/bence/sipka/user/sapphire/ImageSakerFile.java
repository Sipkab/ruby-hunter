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
package bence.sipka.user.sapphire;

import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UncheckedIOException;

import javax.imageio.ImageIO;

import saker.build.file.SakerFileBase;
import saker.build.file.content.ContentDescriptor;
import saker.build.file.content.HashContentDescriptor;

public class ImageSakerFile extends SakerFileBase {
	private BufferedImage image;

	protected BufferedImage getImage() {
		return image;
	}

	public ImageSakerFile(String filename, BufferedImage img) {
		super(filename);
		this.image = img;
	}

	@Override
	public ContentDescriptor getContentDescriptor() {
		try {
			return HashContentDescriptor.hash(getBytesImpl());
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	@Override
	public void writeToStreamImpl(OutputStream os) throws IOException {
		ImageIO.write(getImage(), "png", os);
	}
}