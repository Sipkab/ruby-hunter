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
 * XmlParser.cpp
 *
 *  Created on: 2015 febr. 24
 *      Author: sipka
 */

#include <framework/xml/XmlParser.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/xml/XmlNode.h>
#include <framework/xml/XmlAttributes.h>

#include <gen/xmldecl.h>
#include <gen/xmlcompile.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

static unsigned int getIntBE(unsigned char*& p) {
	unsigned int res = ((unsigned int) p[3]) | (((unsigned int) p[2]) << 8) | (((unsigned int) p[1]) << 16) | (((unsigned int) p[0]) << 24);
	p += 4;
	return res;
}
static unsigned short getShortBE(unsigned char*& p) {
	unsigned short res = ((unsigned short) p[1]) | (((unsigned short) p[0]) << 8);
	p += 2;
	return res;
}

static const uint32 FLAG_DYN_TYPE_EMBEDDED { 0x80000000 };

template<>
void XmlParser::versionParser<1>(EndianInputStream<Endianness::Big>& stream, xml::XmlNode* result, xml::XmlAttributes& attrbuf) {
	stream.deserialize<uint32>(reinterpret_cast<uint32&>(result->dynamicType));
	stream.deserialize<uint32>(reinterpret_cast<uint32&>(result->staticType));
	stream.deserialize<uint32>(reinterpret_cast<uint32&>(result->id));
	stream.deserialize<uint32>(result->childcount);
	stream.deserialize<uint32>(result->attributecount);

	attrbuf.currentCount = 0;
	uint32 attributeblocklen;
	stream.deserialize<uint32>(attributeblocklen);
	unsigned char* attrblockptr = attrbuf.attributeBlock; //(const unsigned char*) stream.read(attributeblocklen, &attrblockout);
	int attrblockout = stream.read(attrblockptr, attributeblocklen);
	ASSERT(attrblockout == attributeblocklen) << "Failed to get all attribute block bytes: " << attrblockout << " - " << attributeblocklen;

	for (unsigned int i = 0; i < result->attributecount; ++i) {
		RXml::Attr attrid = (RXml::Attr) getIntBE(attrblockptr);
		unsigned char attrtype = getShortBE(attrblockptr);
		unsigned int valuelen = getIntBE(attrblockptr);

		attrbuf.add(attrid, attrtype, attrblockptr, valuelen);
		attrblockptr += valuelen;
	}

	const bool embed = HAS_FLAG((uint32 ) result->dynamicType, FLAG_DYN_TYPE_EMBEDDED);

	const XmlReaderFunctionCollection* inflator;

	if (embed) {
		ASSERT(result->parent != nullptr) << "parent is nullptr for embedded child";
		result->dynamicType = (RXml::Elem) ((uint32) result->dynamicType & ~FLAG_DYN_TYPE_EMBEDDED);
		inflator = XML_READER_FUNCTIONS + (uint32) result->dynamicType;
		result->data = XML_READER_FUNCTIONS[(uint32) result->parent->dynamicType].getChildFunction(*result->parent, *result, attrbuf);
	} else {
		inflator = XML_READER_FUNCTIONS + (uint32) result->dynamicType;
		result->data = inflator->inflateFunction(*result, attrbuf);
	}
	ASSERT(result->data != nullptr) << "Inflater returned nullptr";

	if (result->childcount > 0) {
		xml::XmlNode childres;
		childres.param = result->param;
		childres.parent = result;
		for (childres.index = 0; childres.index < result->childcount; ++childres.index) {
			versionParser<1>(stream, &childres, attrbuf);
			const bool accepted = inflator->addChildFunction(*result, childres);
			ASSERT(accepted) << "Xml child wasn't accepted: staticType: " << childres.staticType << ", dynamicType: "
					<< childres.dynamicType << " id: " << childres.id << " at parent: " << result->dynamicType << " id: " << result->id;
		}
	}
}

void XmlParser::parseXml(FileInput& file, xml::XmlNode* result) {
	auto stream = EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(file));
	uint32 version = 0;
	stream.deserialize<uint32>(version);
	switch (version) {
		case 1: {
			uint32 maxAttrCount = 0;
			uint32 maxAttrBlockLength = 0;
			stream.deserialize<uint32>(maxAttrCount);
			stream.deserialize<uint32>(maxAttrBlockLength);
			xml::XmlAttributes attrbuf { maxAttrCount, maxAttrBlockLength };
			result->parent = nullptr;
			versionParser<1>(stream, result, attrbuf);
			break;
		}
		default:
			THROW()<< "Unknown version for xml: " << version;
			break;
		}
	}

xml::XmlNode XmlParser::parseXmlAsset(RAssetFile assetFileId) {
	XmlParser parser;
	xml::XmlNode result;
	parseXmlAsset(parser, assetFileId, &result);
	return result;
}
void XmlParser::parseXmlAsset(RAssetFile assetFileId, xml::XmlNode* result) {
	XmlParser parser;
	parseXmlAsset(parser, assetFileId, result);
}

void XmlParser::parseXmlAsset(XmlParser& parser, RAssetFile assetFileId, xml::XmlNode* result) {
	AssetFileDescriptor xml { assetFileId };
	FileInput* in = xml.createInput();
	in->open();
	parser.parseXml(*in, result);
	in->close();
	delete in;
}

} // namespace rhfw
