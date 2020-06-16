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
 * XmlNode.h
 *
 *  Created on: 2015 febr. 24
 *      Author: sipka
 */

#ifndef XMLNODE_H_
#define XMLNODE_H_

#include <gen/configuration.h>
#include <gen/xmldecl.h>

#include <gen/log.h>

namespace rhfw {

namespace xml {

class XmlNode {
public:
	RXml::Elem dynamicType;
	RXml::Elem staticType;
	RXml::Id id;
	void* data;

	XmlNode* parent;
	unsigned int index;
	unsigned int childcount;
	unsigned int attributecount;

	void* param;

	template<typename T>
	operator T*() const {
		return static_cast<T*>(data);
	}

};

} // namespace xml

#if LOGGING_ENABLED

template<>
inline _tostring_type __internal_tostring<xml::XmlNode>(const xml::XmlNode& value) {
	return TOSTRING(value.dynamicType) + " " + TOSTRING(value.id) + " parent: "
			+ (value.parent == nullptr ? "none" : TOSTRING(value.parent->dynamicType));
}

#endif /* DEBUG */

} // namespace rhfw

#endif /* XMLNODE_H_ */
