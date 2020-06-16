#ifndef RHFW_GEN_ASSETS_H_
#define RHFW_GEN_ASSETS_H_

#include <gen/fwd/types.h>

@include:range_iterator.cpp@

namespace rhfw {

namespace RAssets {

@function exportminmax(list, tabstack):
	@opt Collections.sort(list)@
	@if list.size() == 0:
@String.join("", tabstack)@static __enumerator<RAssetFile, 0x0, 0x0> enumerate() { return { }; }
	@else:
		@if list.get(list.size() - 1) - list.get(0) == list.size() - 1:

@String.join("", tabstack)@static __enumerator<RAssetFile, 0x@Integer.toHexString(list.get(0))@, 0x@Integer.toHexString(list.get(list.size() - 1) + 1)@> enumerate() { return { }; }

		@else:

@String.join("", tabstack)@static __multi_enumerator<RAssetFile

		@var start = list.remove(0)@
		@var end = start@
		@foreach item in list:
			@if item == end + 1: @end = item@ @else:

@String.join("", tabstack)@,__range_iterator<RAssetFile, 0x@Integer.toHexString(start)@, 0x@Integer.toHexString(end+1)@>

			@start = item@
			@end = start@
			@
		@

@String.join("", tabstack)@,__range_iterator<RAssetFile, 0x@Integer.toHexString(start)@, 0x@Integer.toHexString(end+1)@>
@String.join("", tabstack)@> enumerate() { return { }; }

		@
	@
@
@var directory = ""@
@var stack = new Stack()@
@var tabstack = new Stack()@
@void stack.push(new ArrayList())@
@void tabstack.push("\t")@
@foreach item in this.entrySet():
	@var itempath = item.getKey().replaceAll("/(?=[0-9])", "/_").replace(".", "_")@
	@var itemdir = itempath.substring(0, itempath.lastIndexOf("/") + 1)@
	@if !itemdir.equals(directory):
		@while !itemdir.startsWith(directory):
			@directory = directory.substring(0, directory.lastIndexOf("/", directory.length() - 2) + 1)@
			@call exportminmax(stack.pop(), tabstack)@
			@tabstack.pop()@
@String.join("", tabstack)@}
		@
		@while directory.length() < itemdir.length():
			@var nextdir = itemdir.substring(directory.length(), itemdir.indexOf("/", directory.length()))@
			@directory = directory + nextdir + "/"@
@String.join("", tabstack)@namespace @nextdir@ {
			@void stack.push(new ArrayList())@
			@void tabstack.push("\t")@
		@
	@
	@void stack.peek().add(item.getValue())@
	@var remain = itempath.substring(directory.length())@
@String.join("", tabstack)@static const RAssetFile @remain.replaceAll("[^0-9a-zA-Z_:]", "_")@ = (RAssetFile) 0x@Integer.toHexString(item.getValue())@;
@
@call exportminmax(stack.pop(), tabstack)@

@while tabstack.size() > 1:
	@void tabstack.pop()@
@String.join("", tabstack)@}
@call exportminmax(stack.pop(), tabstack)@
@

}  // namespace RAssets

}  // namespace rhfw

#endif
