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
 * FrameAnimation.cpp
 *
 *  Created on: 2016. apr. 29.
 *      Author: sipka
 */

#include <framework/resource/ResourceManager.h>
#include <framework/xml/XmlAttributes.h>
#include <framework/xml/XmlNode.h>
#include <framework/xml/XmlParser.h>
#include <gen/assets.h>
#include <gen/log.h>
#include <gen/resources.h>
#include <gen/xmldecl.h>
#include <sapphire/FrameAnimation.h>

using namespace userapp;
LINK_XML(FrameAnimationElement, FrameAnimation::Element)

namespace rhfw {

template<>
void* inflateElement<RXml::Elem::FrameAnimation>(const xml::XmlNode& node, const xml::XmlAttributes& attributes) {
	userapp::FrameAnimation* res = static_cast<userapp::FrameAnimation*>(node.param);
	res->applyAttributes(node.childcount, attributes);
	return res;
}
template<>
void* getChild<RXml::Elem::FrameAnimation>(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes) {
	userapp::FrameAnimation* thiz = parent;
	ASSERT(child.dynamicType == RXml::Elem::FrameAnimationElement);
	auto&& elemres = thiz->elements[child.index];
	auto* elem = new FrameAnimation::Element(attributes);
	elemres = Resource<FrameAnimation::Element> { new ResourceBlock(elem) };
	return elem;
}
template<>
bool addChild<RXml::Elem::FrameAnimation>(const xml::XmlNode& parent, const xml::XmlNode& child) {
	userapp::FrameAnimation* thiz = parent;
	ASSERT(child.dynamicType == RXml::Elem::FrameAnimationElement);
	return true;
}

}  // namespace rhfw

namespace userapp {

FrameAnimation::Element::Element() {
}
FrameAnimation::Element::Element(const xml::XmlAttributes& attrs) {
	applyAttributes(attrs);
}
void FrameAnimation::Element::applyAttributes(const xml::XmlAttributes& attrs) {
	texture = attrs.get<Resource<render::Texture>>(RXml::Attr::texture);
	pos.left = attrs.get<float>(RXml::Attr::x);
	pos.top = attrs.get<float>(RXml::Attr::y);
	pos.right = pos.left + attrs.get<float>(RXml::Attr::w);
	pos.bottom = pos.top + attrs.get<float>(RXml::Attr::h);
}

FrameAnimation::FrameAnimation(ResId resid)
		: resid(resid) {
}
FrameAnimation::~FrameAnimation() {
}

bool FrameAnimation::load() {
	//TODO error handling on xml
	xml::XmlNode node;
	node.param = this;
	XmlParser::parseXmlAsset(ResourceManager::idToFile(resid), &node);
	//TODO element error check
	for (unsigned int i = 0; i < childCount; ++i) {
		elements[i].load();
	}
	return true;
}

void FrameAnimation::free() {
	for (unsigned int i = 0; i < childCount; ++i) {
		elements[i].free();
	}
	delete[] elements;
}

void FrameAnimation::applyAttributes(unsigned int childcount, const xml::XmlAttributes& attrs) {
	elements = new Resource<Element> [childcount];
	this->childCount = childcount;
}

}  // namespace userapp

