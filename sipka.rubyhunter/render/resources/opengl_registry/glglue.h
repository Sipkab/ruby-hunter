#ifndef GL_GLUE_H_
#define GL_GLUE_H_

#include "glglue_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type definitions
 */
@foreach t in types:
@t@
@
/**
 * End of type definitions
 */

@replace:glglue_enums@

@replace:glglue_commands@

/**
 * Extensions
 */
@foreach ext in extensions:
#ifdef GLGLUE_EXT_@ext.name@
	@if ext.comment != null:
/*
 * @ext.comment@
 */
	@
	@foreach req in ext.requires:
		@foreach cmdstr in req.commands:
			@var cmd = registry.commands.get(cmdstr)@
			@if commands.contains(cmd):
	typedef @cmd.untilname@ (GLGLUE_APIENTRY * GLPROTO_@cmd.name@) (@cmd.getParametersString()@);
			@
		@
	@
#endif /* GLGLUE_EXT_@ext.name@ */
@
/**
 * End of extensions
 */

#ifdef __cplusplus
}
#endif


#endif
