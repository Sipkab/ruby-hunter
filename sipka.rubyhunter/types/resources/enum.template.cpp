/**
 * Generated enumeration class
 */
enum class @this.getName()@ : @this.backingType@ {
@foreach enum in this.values.entrySet() :
	@enum.getKey()@ = @enum.getValue()@, /* 0x@Integer.toHexString(enum.getValue())@ */@
	_count_of_entries = @this.values.size()@
};
