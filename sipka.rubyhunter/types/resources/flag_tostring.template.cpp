template<> ::rhfw::_tostring_type rhfw::__internal_tostring<rhfw::@this.getTypeRepresentation()@>(const rhfw::@this.getTypeRepresentation()@& value) {
	char resbuffer[1024];
	char* ptr = resbuffer;
	snprintf(resbuffer, sizeof(resbuffer), "%s", "NO_FLAG");
@foreach flag in this.values.entrySet():
	@var name = flag.getKey()@
	@if flag.getValue() != 0:
	if(HAS_FLAG(value, rhfw::@this.getName()@::@name@)) { int res = snprintf(ptr, sizeof(resbuffer) - (ptr - resbuffer), ptr == resbuffer ? "%s" : " | %s", "@name@"); ptr += res; }
	@
@
	return resbuffer;
}
