template<> ::rhfw::_tostring_type rhfw::__internal_tostring<rhfw::@this.getTypeRepresentation()@>(const rhfw::@this.getTypeRepresentation()@& value) {
	switch((int) value){
	@foreach enum in this.getIntNameToStringMap().entrySet():
		case @enum.getKey()@: return "@enum.getValue()@";
	@
		default: THROW() << "Invalid enum(@this.getName()@) value: " << (uint32) value; return "(@this.getName()@) Invalid value";
	}
}
