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
 * SapphireMenuElement.cpp
 *
 *  Created on: 2016. apr. 14.
 *      Author: sipka
 */

#include <framework/xml/XmlNode.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireMenuElement.h>

using namespace rhfw;
using namespace userapp;

LINK_XML_SIMPLE(SapphireMenuElement)

namespace userapp {

SapphireMenuElement::SapphireMenuElement()
		: id { RXml::Id::NO_ID } {
}
SapphireMenuElement::SapphireMenuElement(const xml::XmlNode& node, const xml::XmlAttributes& attributes)
		: SapphireMenuElement() {
	applyAttributes(attributes, node.id);
}
SapphireMenuElement::~SapphireMenuElement() {
}

void SapphireMenuElement::applyAttributes(const rhfw::xml::XmlAttributes& attributes, RXml::Id id) {
	text = attributes.get<FixedString>(RXml::Attr::text);
	icon = attributes.get<Resource<FrameAnimation>>(RXml::Attr::icon);
	this->id = id;
}

} // namespace userapp

