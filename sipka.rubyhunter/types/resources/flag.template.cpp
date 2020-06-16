/**
 * Generated flag class
 */
enum class @this.getName()@ : @this.backingType@ {
@foreach flag in this.values.entrySet() :
	@flag.getKey()@ = 0x@Integer.toHexString(flag.getValue())@,@
	_count_of_entries = @this.values.size()@
};
