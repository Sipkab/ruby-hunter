#ifndef GEN_SHADER_@shader.getClassUrl().getExactClassName()@_H_
#define GEN_SHADER_@shader.getClassUrl().getExactClassName()@_H_

#include <framework/render/ShaderPipelineStage.h>
#include <framework/resource/Resource.h>
#include <framework/resource/TrackingResourceBlock.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Vector.h>
#include <framework/utils/utility.h>

namespace rhfw {
namespace render {
	class Texture;
}  // namespace render
class @shader_resource_base_classname@ : public render::ShaderPipelineStage {
private:
@foreach uniform in shader.getUniforms():
public:
	class @uniform.getName()@_value {
	public:
	@foreach member in uniform.getMembers() :
		@member.getType().getFrameworkType()@ @member.getName()@;
	@
	};

	class @uniform.getName()@ : public ShareableResource, public @uniform.getName()@_value {
	public:
		friend class @uniform.getClassUrl().getExactClassName()@;
		@uniform.getName()@& operator=(const @uniform.getName()@_value& v) {
			@uniform.getName()@_value::operator=(v);
			return *this;
		}
		void update(const @uniform.getName()@_value& val){
			*this = val;
			update();
		}
		virtual void update() = 0;
	protected:
		@uniform.getName()@(){}
	};
protected:
	virtual @uniform.getName()@* createUniform_@uniform.getName()@Impl() = 0;
	LinkedList<@uniform.getName()@> uniforms_@uniform.getName()@;
public:
	Resource<@uniform.getName()@> createUniform_@uniform.getName()@(){
		return Resource<@uniform.getName()@> { new TrackingResourceBlock<@uniform.getName()@> {
				createUniform_@uniform.getName()@Impl(), uniforms_@uniform.getName()@
			} };
	}
@

protected:
	virtual void moveResourcesFrom(ShaderPipelineStage&& aprog) override {
		@shader_resource_base_classname@& prog = static_cast<@shader_resource_base_classname@&>(aprog);
		@foreach uniform in shader.getUniforms():
			uniforms_@uniform.getName()@ = util::move(prog.uniforms_@uniform.getName()@);
			for(auto* o : uniforms_@uniform.getName()@.nodes()){
				auto* res = static_cast<TrackingResourceBlock<@uniform.getName()@>*>(o);
				if(res->isLoaded()){
					res->get()->free();
				}
			}
		@
	}
	virtual void updateMovedResources() override {
		//TODO check if uniforms are current
		@foreach uniform in shader.getUniforms():
			for(auto* o : uniforms_@uniform.getName()@.nodes()){
				auto* res = static_cast<TrackingResourceBlock<@uniform.getName()@>*>(o);
				auto* nres = createUniform_@uniform.getName()@Impl();
				auto* oldres = static_cast<@uniform.getName()@*>(res->replace(nres));
				if(res->isLoaded()) {
					nres->load();
				}
				*nres = static_cast<@uniform.getName()@_value&>(*oldres);
				nres->update();
				delete oldres;
			}
		@
	}
	virtual void freeInContext() override {
		@foreach uniform in shader.getUniforms():
			for(auto* o : uniforms_@uniform.getName()@.nodes()){
				auto* res = static_cast<TrackingResourceBlock<@uniform.getName()@>*>(o);
				if(res->isLoaded()){
					res->get()->free();
				}
			}
		@
	}
	virtual bool reloadInNewContext() override {
		//TODO check if uniforms are current
		@foreach uniform in shader.getUniforms():
			for(auto* o : uniforms_@uniform.getName()@.nodes()){
				auto* res = static_cast<TrackingResourceBlock<@uniform.getName()@>*>(o);
				if(res->isLoaded()) {
					res->get()->load();
				}
			}
		@
		return true;
	}
};

} // namespace rhfw
#endif
