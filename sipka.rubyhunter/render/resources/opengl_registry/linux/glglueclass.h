#ifndef GLGLUEFUNCTIONS_LINUX_H_
#define GLGLUEFUNCTIONS_LINUX_H_

#include "glglue.h"
@var registry = this.getRegistry()@

namespace rhfw {
namespace render {
class RenderingContext;
}  // namespace render
}  // namespace rhfw

class GlGlueFunctions {
private:
	void* dlhandle = nullptr;
protected:
	bool load(rhfw::render::RenderingContext* context);
	void free();
public:
	@foreach f in this.features:
		@foreach req in f.requires:
			@foreach cmdstr in req.commands:
				@var cmd = registry.commands.get(cmdstr)@
				@if this.commands.contains(cmd):
private: GLPROTO_@cmd.name@ proto_@cmd.name@ = nullptr;
public:	@cmd.untilname@ @cmd.name@(@cmd.getParametersString()@) { return this->proto_@cmd.name@(@cmd.getParameterNamesString()@); }
				@
			@
		@
	@
};

#endif /* GLGLUEFUNCTIONS_LINUX_H_ */
