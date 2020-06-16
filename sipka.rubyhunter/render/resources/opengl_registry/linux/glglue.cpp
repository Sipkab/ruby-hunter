#include "glglueclass.h"
#include <gen/log.h>
#include <dlfcn.h>

bool GlGlueFunctions::load(rhfw::render::RenderingContext* context) {
	dlhandle = dlopen("libGL.so", RTLD_LAZY);
	if (dlhandle == nullptr) {
		LOGE() << "libGL.so not found";
		dlhandle = dlopen("libGL.so.1", RTLD_LAZY);
		if (dlhandle == nullptr) {
			LOGE() << "libGL.so.1 not found";
			dlhandle = dlopen("libGL.so.1.2.0", RTLD_LAZY);
			if (dlhandle == nullptr) {
				LOGE() << "libGL.so.1.2.0 not found";
				return false;
			}
		}
	}
	ASSERT(dlhandle != nullptr);
	auto* getprocaddr = (void* (*)(const char*))dlsym(dlhandle, "glXGetProcAddress");
	if(getprocaddr == nullptr){
		getprocaddr = (void* (*)(const char*))dlsym(dlhandle, "glXGetProcAddressARB");
	}
	@foreach f in features:
		@foreach req in f.requires:
			@foreach cmdstr in req.commands:
				@var cmd = registry.commands.get(cmdstr)@
				@if commands.contains(cmd):
	proto_@cmd.name@ = (GLPROTO_@cmd.name@) getprocaddr("@cmd.name@");
	ASSERT(proto_@cmd.name@ != nullptr);
				@
			@
		@
	@
	return true;
}
void GlGlueFunctions::free() {
	dlclose(dlhandle);
}
