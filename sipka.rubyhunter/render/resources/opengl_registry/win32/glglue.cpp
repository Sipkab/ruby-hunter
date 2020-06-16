#undef NOGDI
#include <windows.h>

#include "glglueclass.h"
#include <gen/log.h>

#include <win32platform/window/Win32OpenGl30RenderingContext.h>

@var fv10 = registry.features.get("GL_VERSION_1_0")@
@var fv11 = registry.features.get("GL_VERSION_1_1")@

bool GlGlueFunctions::load(rhfw::render::RenderingContext* context) {
	HMODULE module = static_cast<rhfw::render::Win32OpenGl30RenderingContext*>(context)->getLibrary();
	auto wglgetproc = static_cast<rhfw::render::Win32OpenGl30RenderingContext*>(context)->wglfunc_wglGetProcAddress;
	ASSERT(module != NULL);
	@foreach f in features:
		@foreach req in f.requires:
			@foreach cmdstr in req.commands:
				@var cmd = registry.commands.get(cmdstr)@
				@if commands.contains(cmd):
					@if f != fv10 && f != fv11:
	proto_@cmd.name@ = (GLPROTO_@cmd.name@) wglgetproc("@cmd.name@");
					@else:
	proto_@cmd.name@ = (GLPROTO_@cmd.name@) GetProcAddress(module, "@cmd.name@");
					@
	ASSERT(proto_@cmd.name@ != nullptr);
				@
			@
		@
	@
	return true;
}

void GlGlueFunctions::free() {
}
