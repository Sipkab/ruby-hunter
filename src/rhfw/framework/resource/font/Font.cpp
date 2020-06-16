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
 * Font.cpp
 *
 *  Created on: 2014.06.15.
 *      Author: sipka
 */

#include <framework/resource/font/Font.h>
#include <framework/render/Texture.h>
#include <framework/resource/ResourceManager.h>
#include <framework/resource/PackageResource.h>

#include <framework/xml/XmlAttributes.h>
#include <framework/xml/XmlNode.h>
#include <framework/xml/XmlParser.h>

#include <gen/log.h>
#include <gen/types.h>
#include <gen/xmldecl.h>

using namespace rhfw;
LINK_XML(Char, CharDescription)

namespace rhfw {

template<>
void* inflateElement<RXml::Elem::Font>(const xml::XmlNode& node, const xml::XmlAttributes& attrs) {
	Font* res = static_cast<Font*>(node.param);
	res->applyAttributes(attrs, node.childcount);
	return res;
}

template<>
void* getChild<RXml::Elem::Font>(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes) {
	Font* thiz = parent;
	auto* res = thiz->charmap + child.index;
	res->applyAttributes(attributes);
	return res;
}
template<>
bool addChild<RXml::Elem::Font>(const xml::XmlNode& parent, const xml::XmlNode& child) {
	Font* thiz = parent;
	ASSERT(child.index == 0 || thiz->charmap[child.index - 1].id < thiz->charmap[child.index].id)
			<< "Character unicode id is not in ascending order at child index: " << child.index << " charmap[child.index - 1].id: "
			<< (unsigned int) thiz->charmap[child.index - 1].id << " charmap[child.index].id: "
			<< (unsigned int) thiz->charmap[child.index].id << ", characterCount: " << thiz->characterCount;
	return true;
}

bool Font::load() {
	//TODO error handling on xml
	xml::XmlNode node;
	node.param = this;
	XmlParser::parseXmlAsset(ResourceManager::idToFile(resId), &node);
	return texture.load();
}
void Font::free() {
	texture.free();
	delete[] charmap;
}
void Font::applyAttributes(const xml::XmlAttributes& attrs, unsigned int charcount) {
	ASSERT(charcount > 0) << "There are no characters defined for font " << resId;

	this->characterCount = charcount;
	this->charmap = new CharDescription[this->characterCount];

	texture = attrs.get<Resource<render::Texture>>(RXml::Attr::texture);
	loadSize = attrs.get<float>(RXml::Attr::size);
	ascent = attrs.get<float>(RXml::Attr::ascent);
	descent = attrs.get<float>(RXml::Attr::descent);
	leading = attrs.get<float>(RXml::Attr::leading);

	maxHeight = ascent + descent;
}

void CharDescription::applyAttributes(const xml::XmlAttributes& attrs) {
	id = attrs.get<unsigned int>(RXml::Attr::id);

	textureCoordinates.right = attrs.get<float>(RXml::Attr::w);
	textureCoordinates.bottom = attrs.get<float>(RXml::Attr::h);
	textureCoordinates.left = attrs.get<float>(RXml::Attr::x);
	textureCoordinates.top = attrs.get<float>(RXml::Attr::y);

	textureCoordinates.right += textureCoordinates.left;
	textureCoordinates.bottom += textureCoordinates.top;

	xoffset = attrs.get<float>(RXml::Attr::xoffset);
	xAdvance = attrs.get<float>(RXml::Attr::xadvance);
	baseLine = attrs.get<float>(RXml::Attr::baseline);
	//TODO new font metrics input method, XXX
}

CharDescription::CharDescription(const xml::XmlAttributes& attrs) {
	applyAttributes(attrs);
}

const CharDescription& Font::getCharacter(UnicodeCodePoint codepoint) const {
	ASSERT(characterCount > 0) << "There are no characters defined for font " << resId;
	auto* result = getCharacterOptional(codepoint);
	ASSERT(result != nullptr) << "Character not found with codepoint: " << (unsigned int) codepoint;
	return *result;
	/*
	 unsigned int minindex = 0;
	 unsigned int maxindex = characterCount;
	 while (minindex <= maxindex) {
	 unsigned int midindex = (minindex + maxindex) / 2;
	 auto& mid = charmap[midindex];
	 if (mid.id == codepoint)
	 return mid;
	 if (mid.id < codepoint) {
	 minindex = midindex + 1;
	 } else {
	 maxindex = midindex - 1;
	 }
	 }
	 THROW()<< "Character with codepoint not found: " << (unsigned int )codepoint;
	 return charmap[0];
	 */
}

const CharDescription* Font::getCharacterOptional(UnicodeCodePoint codepoint) const {
	int begin = 0;
	int end = characterCount;
	while (begin < end) {
		int midindex = begin + (end - begin) / 2;
		auto& mid = charmap[midindex];
		if (mid.id == codepoint)
			return &mid;
		if (mid.id < codepoint) {
			begin = midindex + 1;
		} else {
			end = midindex;
		}
	}
	return nullptr;
}

} // namespace rhfw

