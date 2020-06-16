#include <gen/resources.h>
template<> ::rhfw::_tostring_type rhfw::__internal_tostring<rhfw::@this.getTypeRepresentation()@>(const rhfw::@this.getTypeRepresentation()@& value) {
	switch(value){
@foreach enum in this.resourcesmap.entrySet():
	@var cppname = enum.getKey().replaceAll("/(?=[0-9])", "::_").replaceAll("/", "::").replaceAll("[^0-9a-zA-Z_:]", "_")@
		case rhfw::ResIds::@cppname@: return "@enum.getKey()@";
@
		default: THROW() << "Invalid enum(@this.getName()@) value: " << (uint32) value; return "(@this.getName()@) Invalid value";
	}
}
