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
/*
 * Font.h
 *
 *  Created on: 2014.06.15.
 *      Author: sipka
 */

#ifndef FONT_H_
#define FONT_H_

#include <framework/render/Texture.h>
#include <framework/io/key/UnicodeCodePoint.h>
#include <framework/geometry/Vector.h>
#include <framework/geometry/Rectangle.h>
#include <framework/resource/Resource.h>

#include <gen/types.h>
#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/xmldecl.h>
#include <gen/resources.h>

namespace rhfw {

namespace xml {
class XmlAttributes;
}  // namespace xml

class CharDescription {
public:
	UnicodeCodePoint id;
	Rectangle textureCoordinates;
	//amount from the top of the char
	float baseLine = 0.0f;
	float xoffset = 0.0f;
	float xAdvance = 0.0f;

	CharDescription(const xml::XmlAttributes& attrs);

	CharDescription() {
	}

	void applyAttributes(const xml::XmlAttributes& attrs);

	float getWidth() const {
		return textureCoordinates.width();
	}
	float getHeight() const {
		return textureCoordinates.height();
	}

};

class Font: public ShareableResource {
	friend void* rhfw::inflateElement<RXml::Elem::Font>(const xml::XmlNode& node, const xml::XmlAttributes& attrs);
	friend void* rhfw::getChild<RXml::Elem::Font>(const xml::XmlNode& parent, const xml::XmlNode& child,
			const xml::XmlAttributes& attributes);
	friend bool rhfw::addChild<RXml::Elem::Font>(const xml::XmlNode& parent, const xml::XmlNode& child);
protected:
	Resource<render::Texture> texture;

	float loadSize = 0.0f;
	float ascent = 0.0f;
	float descent = 0.0f;
	float leading = 0.0f;

	float maxHeight = 0.0f;

	unsigned int characterCount = 0;
	CharDescription* charmap = nullptr;

	ResId resId;

	bool load() override;
	void free() override;

	void applyAttributes(const xml::XmlAttributes& attrs, unsigned int charcount);
public:
	explicit Font(ResId resid)
			: resId { resid } {
	}
	virtual ~Font() {
	}

	render::Texture& getTexture() {
		return texture;
	}

	/**
	 * Returns the difference between lines.
	 */
	float getLeading(float textsize) const {
		return leading * textsize / maxHeight;
	}

	const CharDescription& operator[](UnicodeCodePoint index) const {
		return getCharacter(index);
	}
	const CharDescription* getCharacterOptional(UnicodeCodePoint codepoint) const;
	const CharDescription& getCharacter(UnicodeCodePoint codepoint) const;
	bool hasCharacter(UnicodeCodePoint codepoint) const {
		return getCharacterOptional(codepoint) != nullptr;
	}

	template<typename Iterable>
	float measureText(Iterable&& s, float size) const {
		float x = 0.0f;
		const float sizeFactor = size / maxHeight;
		for (auto&& c : s) {
			x += getCharacter(c).xAdvance;
		}
		x *= sizeFactor;
		return x;
	}

	/**
	 * Measures the text given the input, and the size in pixels.
	 * Returns the width of the text.
	 */
	float measureText(const char* s, float size) const {
		float x = 0.0f;
		const float sizeFactor = size / maxHeight;
		while (*s) {
			x += getCharacter(*s).xAdvance;
			++s;
		}
		x *= sizeFactor;
		return x;
	}

	/**
	 * Measures the text given the input, and the size in pixels.
	 * Returns the width of the text.
	 */
	template<typename CharIt>
	float measureText(CharIt s, CharIt end, float size) const {
		float x = 0.0f;
		const float sizeFactor = size / maxHeight;
		while (s != end) {
			x += getCharacter(*s).xAdvance;
			++s;
		}
		x *= sizeFactor;
		return x;
	}

	/**
	 * Same as float measureText(const char* s, float size),
	 * but returns the length of the text in the parameter length.
	 */
	template<typename CharIt>
	float measureText(CharIt s, float size, unsigned int* length) const {
		float x = 0.0f;
		*length = 0;
		const float sizeFactor = size / maxHeight;
		while (*s) {
			x += getCharacter(*s).xAdvance;
			++s;
			++(*length);
		}
		x *= sizeFactor;
		return x;
	}
	/**
	 * Measures how many characters fit in the given width (maxWidth argument).
	 * Return value points AFTER the last character that fits.
	 * if *result == 0, then all of the string fits, and the result points to the terminating null character.
	 */
	template<typename CharIt>
	CharIt measureText(CharIt s, float size, float maxWidth) const {
		float x = 0.0f;
		const float sizeFactor = size / maxHeight;
		maxWidth /= sizeFactor;
		while (*s) {
			x += getCharacter(*s).xAdvance;
			if (x > maxWidth)
				break;
			++s;
		}
		return s;
	}
	/**
	 * Same as measureText(CharIt s, float size, float maxWidth), but returns the actual length of the string
	 */
	template<typename CharIt>
	CharIt measureText(CharIt s, float size, float maxWidth, float* outwidth) const {
		float x = 0.0f;
		const float sizeFactor = size / maxHeight;
		maxWidth /= sizeFactor;
		while (*s) {
			x += getCharacter(*s).xAdvance;
			if (x > maxWidth)
				break;
			*outwidth = x;
			++s;
		}
		*outwidth *= sizeFactor;
		return s;
	}

	/**
	 * Fills buffer with font rendering data. Coordinate system origin is at top left. startx and starty points to the left baseline start of the string
	 */
	template<typename CharIt, typename VertexInput, typename SetFunc>
	void fillBufferDataWithCharacters(CharIt it, CharIt end, VertexInput* input, float size, const Vector2F& position, Vector2F startpos,
			const SetFunc& setter) const {
		const float sizeFactor = size / maxHeight;
		while (it != end) {
			UnicodeCodePoint cp = (UnicodeCodePoint) *it;
			++it;
			const CharDescription& chardesc = getCharacter(cp);

			const float x = startpos.x() - sizeFactor * chardesc.xoffset;
			const float y = startpos.y() - sizeFactor * chardesc.baseLine;
			const float w = sizeFactor * chardesc.getWidth();
			const float h = sizeFactor * chardesc.getHeight();

			startpos.x() += sizeFactor * chardesc.xAdvance;

			setter(input[0], Vector2F { x, y + h }, chardesc.textureCoordinates.leftBottom()); //bal also
			setter(input[1], Vector2F { x + w, y + h }, chardesc.textureCoordinates.rightBottom()); //jobb also
			setter(input[2], Vector2F { x, y }, chardesc.textureCoordinates.leftTop()); //bal felso
			setter(input[3], Vector2F { x + w, y }, chardesc.textureCoordinates.rightTop()); //jobb felso

			input += 4;
		}
	}

	/**
	 * Fills buffer with font rendering data. Coordinate system origin is at top left.
	 */
	template<typename CharIt, typename VertexInput, typename SetFunc>
	void fillBufferDataWithCharacters(CharIt it, CharIt end, VertexInput* input, float size, const Vector2F& position, Gravity gravity,
			const SetFunc& setter) const {
		Vector2F start;
		switch (gravity & Gravity::MASK_HORIZONTAL) {
			case Gravity::LEFT:
				start.x() = position.x();
				break;
			case Gravity::CENTER_HORIZONTAL:
				start.x() = position.x() - measureText(it, end, size) / 2.0f;
				break;
			case Gravity::RIGHT:
				start.x() = position.x() - measureText(it, end, size);
				break;
			default: {
				THROW()<< "Invalid horizontal gravity flag: " << gravity;
				return;
			}
		}
		switch (gravity & Gravity::MASK_VERTICAL) {
			case Gravity::TOP:
				start.y() = position.y() + ascent * size / maxHeight;
				break;
			case Gravity::CENTER_VERTICAL:
				start.y() = position.y() - size / 2.0f + ascent * size / maxHeight;
				break;
			case Gravity::BOTTOM:
				start.y() = position.y() - descent * size / maxHeight;
				break;
			case Gravity::BASELINE:
				start.y() = position.y();
				break;
			default: {
				THROW()<< "Invalid horizontal gravity flag: " << gravity;
				return;
			}
		}

		fillBufferDataWithCharacters(it, end, input, size, position, start, setter);
	}

	template<typename CharIt, typename VertexInput, typename SetFunc>
	void fillBufferDataWithCharacters(CharIt it, CharIt end, VertexInput* input, float size, const Vector2F& position, Gravity gravity,
			float measuredwidth, const SetFunc& setter) const {
		Vector2F start;
		switch (gravity & Gravity::MASK_HORIZONTAL) {
			case Gravity::LEFT:
				start.x() = position.x();
				break;
			case Gravity::CENTER_HORIZONTAL:
				start.x() = position.x() - measuredwidth / 2.0f;
				break;
			case Gravity::RIGHT:
				start.x() = position.x() - measuredwidth;
				break;
			default: {
				THROW()<< "Invalid horizontal gravity flag: " << gravity;
				return;
			}
		}
		switch (gravity & Gravity::MASK_VERTICAL) {
			case Gravity::TOP:
				start.y() = position.y() + ascent * size / maxHeight;
				break;
			case Gravity::CENTER_VERTICAL:
				start.y() = position.y() - size / 2.0f + ascent * size / maxHeight;
				break;
			case Gravity::BOTTOM:
				start.y() = position.y() - descent * size / maxHeight;
				break;
			case Gravity::BASELINE:
				start.y() = position.y();
				break;
			default: {
				THROW()<< "Invalid horizontal gravity flag: " << gravity;
				return;
			}
		}

		fillBufferDataWithCharacters(it, end, input, size, position, start, setter);
	}
};
}

#endif /* FONT_H_ */
