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
 * SapphireMenuElement.h
 *
 *  Created on: 2016. apr. 14.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIREMENUELEMENT_H_
#define TEST_SAPPHIRE_SAPPHIREMENUELEMENT_H_

#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/xml/XmlAttributes.h>
#include <gen/xmldecl.h>
#include <sapphire/FrameAnimation.h>

namespace userapp {

class SapphireMenuElement {
public:
	rhfw::AutoResource<FrameAnimation> icon;
	rhfw::FixedString text;

	rhfw::RXml::Id id;

	SapphireMenuElement();
	SapphireMenuElement(const xml::XmlNode& node, const xml::XmlAttributes& attributs);
	void applyAttributes(const rhfw::xml::XmlAttributes& attributes, rhfw::RXml::Id id);
	virtual ~SapphireMenuElement();
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIREMENUELEMENT_H_ */
