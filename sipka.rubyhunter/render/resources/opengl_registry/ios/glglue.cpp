#include "glglueclass.h"
#include <gen/log.h>
#include <dlfcn.h>

bool GlGlueFunctions::load(rhfw::render::RenderingContext* context) {
	dlhandle = dlopen("/System/Library/Frameworks/OpenGLES.framework/OpenGLES", RTLD_LAZY);
	ASSERT(dlhandle != nullptr);
	@foreach f in features:
		@foreach req in f.requires:
			@foreach cmdstr in req.commands:
				@var cmd = registry.commands.get(cmdstr)@
				@if commands.contains(cmd):
	proto_@cmd.name@ = (GLPROTO_@cmd.name@) dlsym(dlhandle, "@cmd.name@");
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
