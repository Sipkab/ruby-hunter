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
package bence.sipka.user.font;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.font.TextLayout;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Collection;
import java.util.stream.Collectors;

import bence.sipka.user.font.FontConverterTaskFactory.FontPackFailedException;

public class FontDescription {
	private Collection<CharRect> characters = null;
	private float size;
	private float ascent;
	private float descent;
	private float leading;

	private BufferedImage chardescImage;
	private int minCodePoint;
	private int maxCodePoint;
	private Font font;
	private Font usedFont;

	private int imageDimension = 1024;

	private boolean debug = false;

	public FontDescription(Font font, int mincodepoint, int maxcodepoint) {
		this.font = font;
		this.minCodePoint = mincodepoint;
		this.maxCodePoint = maxcodepoint;
	}

	public FontDescription(Font font) {
		this(font, Character.MIN_CODE_POINT, Character.MAX_CODE_POINT);
	}

	public float getSize() {
		return size;
	}

	public float getAscent() {
		return ascent;
	}

	public float getDescent() {
		return descent;
	}

	public float getLeading() {
		return leading;
	}

	public int getImageDimension() {
		return imageDimension;
	}

	public boolean isDebug() {
		return debug;
	}

	public void setDebug(boolean debug) {
		this.debug = debug;
	}

	private CharRect getCharacterDescriptor(int[] codepoint, Font font) {
		String chstr = new String(codepoint, 0, 1);

		int imgdim = chardescImage.getWidth();
		if (imgdim < font.getSize2D() * 3) {
			imgdim = Math.round(font.getSize2D() * 3);
			chardescImage = new BufferedImage(imgdim, imgdim, BufferedImage.TYPE_INT_ARGB);
		}
		float halfdim = imgdim / 2.0f;

		Graphics2D g = chardescImage.createGraphics();

		g.setBackground(new Color(0, 0, 0, 0));
		g.clearRect(0, 0, imgdim, imgdim);

		FontConverterTaskFactory.setupGraphics(g, font);

		g.drawString(chstr, halfdim, halfdim);

		TextLayout lay = new TextLayout(chstr, font, g.getFontRenderContext());
		float advance = lay.getAdvance();

		g.dispose();

		int left = imgdim;
		int top = imgdim;
		int right = -1;
		int bottom = -1;
		outer_find_left:

		for (int x = 0; x < imgdim; x++) {
			for (int y = 0; y < imgdim; y++) {
				int argb = chardescImage.getRGB(x, y);
				if (argb != 0) {
					left = x;
					break outer_find_left;
				}
			}
		}
		outer_find_top:

		for (int y = 0; y < imgdim; y++) {
			for (int x = 0; x < imgdim; x++) {
				int argb = chardescImage.getRGB(x, y);
				if (argb != 0) {
					top = y;
					break outer_find_top;
				}
			}
		}
		outer_find_right:

		for (int x = imgdim - 1; x >= 0; x--) {
			for (int y = 0; y < imgdim; y++) {
				int argb = chardescImage.getRGB(x, y);
				if (argb != 0) {
					right = x;
					break outer_find_right;
				}
			}
		}
		outer_find_bottom:

		for (int y = imgdim - 1; y >= 0; y--) {
			for (int x = 0; x < imgdim; x++) {
				int argb = chardescImage.getRGB(x, y);
				if (argb != 0) {
					bottom = y;
					break outer_find_bottom;
				}
			}
		}

		Rectangle2D.Float rect;
		if (right < left) {
			rect = new Rectangle2D.Float();
		} else {
			rect = new Rectangle2D.Float(left - halfdim, top - halfdim, right - left + 1, bottom - top + 1);
			this.ascent = Math.max(this.ascent, halfdim - top + 1);
			this.descent = Math.max(this.descent, bottom - halfdim + 1);
		}
		return new CharRect(chstr, rect, advance, halfdim - left);
	}

	private void drawCharacters(Graphics2D g) {
		for (CharRect cr : getCharacters()) {
			g.drawString(cr.str, (float) cr.resultPos.getX(), (float) cr.resultPos.getY());
		}
	}

	private void packCharacters(Collection<CharRect> chars) throws FontPackFailedException {
		Collection<CharRect> charmap = chars.stream().sorted().collect(Collectors.toList());
		float left = 0.0f;
		float top = 0.0f;
		float maxh = 0.0f;
		for (CharRect cr : charmap) {
			if (left + cr.dispRect.getWidth() > imageDimension) {
				left = 0.0f;
				top += maxh;
				maxh = 0.0f;
			}
			if (top + cr.dispRect.getHeight() > imageDimension) {
				throw new FontPackFailedException("Failed to pack char: " + cr.codepoint);
			}

			cr.resultPos = new Point2D.Double(left + cr.padding + cr.xoffset, top - cr.charRect.getY() + cr.padding);

			left += cr.dispRect.getWidth();
			maxh = (float) Math.max(maxh, cr.dispRect.getHeight());

		}
	}

	public synchronized void initialize() throws FontPackFailedException {
		if (getCharacters() != null) {
			return;
		}
		ArrayList<CharRect> chars;
		float size = 40.0f;
		while (true) {
			try {
				chars = new ArrayList<>();
				Font font = this.font.deriveFont(size);
				int bufimgdim = (int) (font.getSize2D() * 3.0f);
				chardescImage = new BufferedImage(bufimgdim, bufimgdim, BufferedImage.TYPE_INT_ARGB);
				for (int[] c = new int[] { minCodePoint }; c[0] <= maxCodePoint; c[0]++) {
					if (font.canDisplay(c[0])) {
						chars.add(getCharacterDescriptor(c, font));
					}
				}
				Graphics2D g = chardescImage.createGraphics();
				TextLayout lay = new TextLayout("a\nb", font, g.getFontRenderContext());
				g.dispose();
				this.size = font.getSize2D();
				this.leading = lay.getLeading();
				chardescImage = null;

				packCharacters(chars);
				usedFont = font;
				characters = chars;
				size = size + 1;
				if (debug) {
					break;
				}
			} catch (FontPackFailedException e) {
				if (getCharacters() != null) {
					break;
				}
				size = size - 1;
				if (size <= 0.0f) {
					throw new FontPackFailedException("Failed to convert font to png: " + font.getFontName(), e);
				}
			}
		}
	}

	public BufferedImage getImage() {
		try {
			initialize();
			BufferedImage nimg = new BufferedImage(imageDimension, imageDimension, BufferedImage.TYPE_INT_ARGB);
			Graphics2D g = nimg.createGraphics();
			FontConverterTaskFactory.setupGraphics(g, usedFont);
			drawCharacters(g);
			g.dispose();
			return nimg;
		} catch (FontPackFailedException e) {
			throw new RuntimeException(e);
		}
	}

	public Collection<CharRect> getCharacters() {
		return characters;
	}
}