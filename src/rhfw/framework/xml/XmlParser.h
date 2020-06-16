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
 * XmlParser.h
 *
 *  Created on: 2015 febr. 24
 *      Author: sipka
 */

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <framework/xml/XmlNode.h>

#include <gen/configuration.h>
#include <gen/assets.h>

namespace rhfw {

template<Endianness ENDIAN>
class EndianInputStream;
class InputStream;
class FileDescriptor;
class FileInput;
namespace xml {

class XmlAttributes;

}  // namespace xml

class XmlParser {
private:
	template<int version>
	void versionParser(EndianInputStream<Endianness::Big>& stream, xml::XmlNode* result, xml::XmlAttributes& attrbuf);
public:

	XmlParser() {
	}
	~XmlParser() {
	}

	void parseXml(FileInput& file, xml::XmlNode* result);

	static xml::XmlNode parseXmlAsset(RAssetFile resourceId);
	static void parseXmlAsset(RAssetFile resourceId, xml::XmlNode* result);
	static void parseXmlAsset(XmlParser& parser, RAssetFile resourceId, xml::XmlNode* result);

};
}

#endif /* XMLPARSER_H_ */
