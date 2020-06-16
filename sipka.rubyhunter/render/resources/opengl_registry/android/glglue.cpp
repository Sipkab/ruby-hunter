#include "glglueclass.h"
#include <gen/log.h>

#include <EGL/egl.h>

bool GlGlueFunctions::load(rhfw::render::RenderingContext* context) {
	@foreach f in features:
		@foreach req in f.requires:
			@foreach cmdstr in req.commands:
				@var cmd = registry.commands.get(cmdstr)@
				@if commands.contains(cmd):
	proto_@cmd.name@ = (GLPROTO_@cmd.name@) eglGetProcAddress("@cmd.name@");
	ASSERT(proto_@cmd.name@ != nullptr);
				@
			@
		@
	@
	return true;
}
void GlGlueFunctions::free() {
}
