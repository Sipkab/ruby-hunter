#ifndef XMLCOMPILE_H_
#define XMLCOMPILE_H_
#include <gen/configuration.h>
#include <gen/fwd/types.h>
namespace rhfw {
namespace RXmlCompile {
namespace Types {
typedef uint16 XmlType;
template<typename T> class __xml_type_deserializer_check{ public: static void assert(XmlType type) { } };

@var i = 0@
@while i < this.serializers.size():
template<> class __xml_type_deserializer_check<@this.serializers.get(i).getTypeRepresentation()@> { public: static void assert(XmlType type) { ASSERT(type == @i@) << type; } };
static const XmlType @this.toCppName(this.serializers.get(i).getName())@ = @i@;
	@i = i + 1@
@


} // namespace Types
} // namespace RXmlCompile
} // namespace rhfw
#endif /* XMLCOMPILE_H_ */
