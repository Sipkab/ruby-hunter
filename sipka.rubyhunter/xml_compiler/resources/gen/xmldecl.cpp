#include <gen/xmldecl.h>
#include <gen/log.h>

namespace rhfw {

@if noinflate:
	@foreach elem in inflate_lut.entrySet() :
			@var elemname = elem.getValue()@
template<>
void* inflateElement<RXml::Elem::@elemname@>(const xml::XmlNode& node, const xml::XmlAttributes& attributes){
	return nullptr;
}
template<>
void* getChild<RXml::Elem::@elemname@>(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes){
	return nullptr;
}
template<>
bool addChild<RXml::Elem::@elemname@>(const xml::XmlNode& parent, const xml::XmlNode& child){
	return false;
}
	@
@

XmlReaderFunctionCollection XML_READER_FUNCTIONS[] = {
	@foreach elem in inflate_lut.entrySet() :
		@var elemname = elem.getValue()@
	{ /* @elemname@ == @elem.getKey()@ */
		inflateElement<RXml::Elem::@elemname@>,
		getChild<RXml::Elem::@elemname@>,
		addChild<RXml::Elem::@elemname@>,
	},
	@
};


}  // namespace rhfw

#if LOGGING_ENABLED

@func echo_tostring(collection, name):
template<> ::rhfw::_tostring_type rhfw::__internal_tostring<rhfw::RXml::@name@>(const rhfw::RXml::@name@& value) {
	switch(value){
	@foreach enum in collection.entrySet():
		case rhfw::RXml::@name@::@enum.getKey()@: return "RXml::@name@::@enum.getKey()@";
	@
		default: THROW() << "Invalid enum(RXml::@name@) value: " << (uint32) value; return "(RXml::@name@) Invalid value";
	}
}
@

@call echo_tostring(attributes, "Attr")@
@call echo_tostring(identifiers, "Id")@
@call echo_tostring(elements, "Elem")@

#endif /* LOGGING_ENABLED */
