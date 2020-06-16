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
 * XmlAttributes.h
 *
 *  Created on: 2016. marc. 21.
 *      Author: sipka
 */

#ifndef XMLATTRIBUTES_H_
#define XMLATTRIBUTES_H_

#include <framework/utils/utility.h>
#include <framework/utils/MemoryInput.h>
#include <framework/io/stream/InputStream.h>

#include <gen/log.h>
#include <gen/xmlcompile.h>
#include <gen/xmldecl.h>
#include <gen/serialize.h>
#include <gen/types.h>

namespace rhfw {
class XmlParser;
namespace xml {

class XmlAttributes {
private:
	class Attribute {
	private:
		friend class XmlAttributes;

		RXml::Attr attrId = (RXml::Attr) -1;

		RXmlCompile::Types::XmlType type = -1;
		const unsigned char* ptr = nullptr;
		unsigned int byteCount = 0;

		Attribute() = default;
		Attribute(RXml::Attr attr, RXmlCompile::Types::XmlType type, const unsigned char* ptr, unsigned int bytecount)
				: attrId { attr }, type { type }, ptr { ptr }, byteCount { bytecount } {
		}
	public:
		template<typename T>
		bool parse(T&& out) const {
			//RXmlCompile::Types::__xml_type_deserializer_check<T>::assert(type);
			MemoryInput<const unsigned char> in { ptr, byteCount };
			auto is = EndianInputStream<Endianness::Big>::wrap(in);
			return is.deserialize<typename util::remove_reference<T>::type>(out);
		}
	};

	friend class rhfw::XmlParser;
	unsigned int currentCount = 0;

	const unsigned int attrCount;
	Attribute* attrs;
	unsigned char* attributeBlock;

	void add(RXml::Attr attr, RXmlCompile::Types::XmlType type, const unsigned char* ptr, unsigned int bytecount) {
		attrs[currentCount++] = {attr, type, ptr, bytecount};
	}
public:
	XmlAttributes(unsigned int attrcount, unsigned int attributeblocksize)
	: attrCount {attrcount} {
		attrs = new Attribute[attrcount];
		attributeBlock = new unsigned char[attributeblocksize];
	}
	~XmlAttributes() {
		delete[] attrs;
		delete[] attributeBlock;
	}

	template<typename T, typename Factory>
	void getOrFactory(RXml::Attr attr, T&& out, const Factory& f) const {
		//TODO binary search
		for (unsigned int i = 0; i < currentCount; ++i) {
			if (attrs[i].attrId == attr) {
				bool success = attrs[i].parse(util::forward<T>(out));
				if(success) {
					return;
				} else {
					//break loop, return with factory method
					break;
				}
			}
		}
		f(util::forward<T>(out));
	}
	template<typename T>
	bool get(RXml::Attr attr, T&& out) const {
		//TODO binary search
		for (unsigned int i = 0; i < currentCount; ++i) {
			if (attrs[i].attrId == attr) {
				return attrs[i].parse(util::forward<T>(out));
			}
		}
		return false;
	}
	template<typename T>
	T get(RXml::Attr attr) const {
		//TODO binary search
		for (unsigned int i = 0; i < currentCount; ++i) {
			if (attrs[i].attrId == attr) {
				T res;
				bool success = attrs[i].parse(res);
				if(success) {
					return util::move(res);
				} else {
					//break loop, return with default value
					break;
				}
			}
		}
		THROW() << "Failed to get attribute value: " << attr;
		return T {};
	}

	template<typename T, typename Factory>
	T getOrFactory(RXml::Attr attr, Factory&& f) const {
		//TODO binary search
		for (unsigned int i = 0; i < currentCount; ++i) {
			if (attrs[i].attrId == attr) {
				T res;
				bool success = attrs[i].parse(res);
				if(success) {
					return util::move(res);
				} else {
					//break loop, return with default value
					break;
				}
			}
		}
		return f();
	}
	template<typename T, typename TParam>
	T getOrDefault(RXml::Attr attr, TParam&& t) const {
		//TODO binary search
		for (unsigned int i = 0; i < currentCount; ++i) {
			if (attrs[i].attrId == attr) {
				T res;
				bool success = attrs[i].parse(res);
				if(success) {
					return util::move(res);
				} else {
					//break loop, return with default value
					break;
				}
			}
		}
		return util::forward<TParam>(t);
	}
};

}
 // namespace xml
}// namespace rhfw

#endif /* XMLATTRIBUTES_H_ */
