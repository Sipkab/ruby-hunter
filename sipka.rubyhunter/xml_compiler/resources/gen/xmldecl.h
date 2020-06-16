#ifndef XMLDECL_H_
#define XMLDECL_H_

#include <framework/utils/utility.h>
#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {
namespace RXml {
enum class Attr : uint32 {
@foreach item in attributes.entrySet() :
	@item.getKey()@ = 0x@Integer.toHexString(item.getValue())@,
@
	_count_of_entries = @attributes.size()@
};
enum class Elem : uint32 {
@foreach item in elements.entrySet() :
	@item.getKey()@ = 0x@Integer.toHexString(item.getValue())@,
@
	_count_of_entries = @elements.size()@
};
enum class Id : uint32 {
@foreach item in identifiers.entrySet() :
	@item.getKey()@ = 0x@Integer.toHexString(item.getValue())@,
@
	_count_of_entries = @identifiers.size()@
};
}  // namespace RXml

namespace xml {
class XmlAttributes;
class XmlNode;
}

template<RXml::Elem Element>
void* inflateElement(const xml::XmlNode& node, const xml::XmlAttributes& attributes);
template<RXml::Elem Element>
void* getChild(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes);
template<RXml::Elem Element>
bool addChild(const xml::XmlNode& parent, const xml::XmlNode& child);

#define LINK_XML(xmlelem, TYPE) \
class __ElementInflaterHelper##xmlelem {\
private:\
	template<typename U>\
	struct has_getchild{\
		typedef long long VALUE_YES; typedef char VALUE_NO;\
		template<typename V, decltype(&V::getXmlChild)* = nullptr> static VALUE_YES test(int*); \
		template<typename V> static VALUE_NO test(void*); \
		static const bool value = sizeof(test<U>(static_cast<int*>(nullptr))) == sizeof(VALUE_YES);\
	};\
	template<typename U>\
	struct has_addchild{\
		typedef long long VALUE_YES; typedef char VALUE_NO;\
		template<typename V, decltype(&V::addXmlChild)* = nullptr> static VALUE_YES test(int*); \
		template<typename V> static VALUE_NO test(void*); \
		static const bool value = sizeof(test<U>(static_cast<int*>(nullptr))) == sizeof(VALUE_YES);\
	};\
public:\
	\
	template<typename U = TYPE, \
			 typename util::enable_if<util::is_constructible<U, const xml::XmlNode&, const xml::XmlAttributes&>::value>::type* = nullptr> \
	static void* inflateElement(const xml::XmlNode& node, const xml::XmlAttributes& attributes) { \
		return new U ( node, attributes );\
	} \
	template<typename U = TYPE, \
		typename util::enable_if<util::is_constructible<U, const xml::XmlAttributes&>::value>::type* = nullptr> \
	static void* inflateElement(const xml::XmlNode& node, const xml::XmlAttributes& attributes) { \
		return new U ( attributes );\
	} \
	template<typename U = TYPE, \
		typename util::enable_if<\
			util::is_constructible<U>::value && \
			!util::is_constructible<U, const xml::XmlAttributes&>::value && \
			!util::is_constructible<U, const xml::XmlNode&, const xml::XmlAttributes&>::value\
			>::type* = nullptr> \
	static void* inflateElement(const xml::XmlNode& node, const xml::XmlAttributes& attributes) { \
		return new U ( );\
	} \
	template<typename U = TYPE, \
			 typename util::enable_if<has_getchild<U>::value>::type* = nullptr> \
	static void* getChild(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes) { \
		return static_cast<U*>(parent)->getXmlChild(child, attributes); \
	} \
	template<typename U = TYPE, \
		typename util::enable_if<!has_getchild<U>::value>::type* = nullptr> \
	static void* getChild(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes) { \
		LOGW()<< "Inflating element instead of getting " << child; \
		return inflateElement<U>(child, attributes); \
	} \
	template<typename U = TYPE, \
		typename util::enable_if<has_addchild<U>::value>::type* = nullptr> \
	static bool addChild(const xml::XmlNode& parent, const xml::XmlNode& child) { \
		return static_cast<U*>(parent)->addXmlChild(child); \
	} \
	template<typename U = TYPE, \
		typename util::enable_if<!has_addchild<U>::value>::type* = nullptr> \
	static bool addChild(const xml::XmlNode& parent, const xml::XmlNode& child) { \
		LOGW()<< "Ignoring XML child " << child; \
		return false; \
	} \
};\
template<> void* ::rhfw::inflateElement<::rhfw::RXml::Elem::xmlelem>(const ::rhfw::xml::XmlNode& node, const ::rhfw::xml::XmlAttributes& attributes)\
	{ return __ElementInflaterHelper##xmlelem::inflateElement(node, attributes); }\
template<> void* ::rhfw::getChild<::rhfw::RXml::Elem::xmlelem>(const ::rhfw::xml::XmlNode& parent, const ::rhfw::xml::XmlNode& child, const ::rhfw::xml::XmlAttributes& attributes)\
	{ return __ElementInflaterHelper##xmlelem::getChild(parent, child, attributes); }\
template<> bool ::rhfw::addChild<::rhfw::RXml::Elem::xmlelem>(const ::rhfw::xml::XmlNode& parent, const ::rhfw::xml::XmlNode& child)\
	{ return __ElementInflaterHelper##xmlelem::addChild(parent, child); }

//TODO add static assertions in addchild and getchild, when xmls contain those, in res.cpp
#define LINK_XML_SIMPLE(type) LINK_XML(type, type)

class XmlReaderFunctionCollection {
public:
	void* (*inflateFunction)(const xml::XmlNode& node, const xml::XmlAttributes& attributes);
	void* (*getChildFunction)(const xml::XmlNode& parent, const xml::XmlNode& child, const xml::XmlAttributes& attributes);
	bool (*addChildFunction)(const xml::XmlNode& parent, const xml::XmlNode& child);
};

extern XmlReaderFunctionCollection XML_READER_FUNCTIONS[];

}  // namespace rhfw

#include <gen/log.h>
#include <framework/utils/utility.h>
#include <framework/xml/XmlNode.h>

#endif /* XMLDECL_H_ */
