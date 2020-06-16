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

import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;

public class CharRect implements Comparable<CharRect> {
	int codepoint;
	String str;
	/**
	 * The rectangle dimensions of the character. Drawn at [0, 0], these are the dimensions the character will take.
	 */
	Rectangle2D charRect;
	Rectangle2D dispRect;
	Point2D resultPos;
	float advance;
	double padding;
	float xoffset;

	public CharRect(String str, Rectangle2D rect, float advance, float xoffset) {
		this.codepoint = str.codePointAt(0);
		this.str = str;
		this.charRect = rect;
		this.advance = advance;
		this.xoffset = xoffset;

		this.padding = 2.0d;
		dispRect = new Rectangle2D.Double(rect.getX() - padding, rect.getY() - padding, rect.getWidth() + padding * 2, rect.getHeight() + padding * 2);
	}

	@Override
	public int compareTo(CharRect o) {
		int result = -Double.compare(charRect.getWidth() * charRect.getHeight(), o.charRect.getWidth() * o.charRect.getHeight());
		return result == 0 ? Integer.compare(codepoint, o.codepoint) : result;
	}
}