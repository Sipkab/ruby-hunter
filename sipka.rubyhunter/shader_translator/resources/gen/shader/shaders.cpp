#include <gen/shader/shaders.h>
#include <gen/types.h>

#include <framework/render/ShaderProgram.h>
#include <framework/render/ShaderPipelineStage.h>

#include <gen/log.h>

namespace rhfw {
template<RenderConfig Renderer, UnifiedShaders Shader>
static render::ShaderProgram* instantiateShader(render::Renderer*);

template<RenderConfig Renderer, UnifiedShaderPipelineStage Pipeline>
static render::ShaderPipelineStage* instantiatePipeline(render::Renderer*);
} // namespace rhfw

@foreach renderconf in rendererconfigs:
	@foreach program in programnames:
#include <gen/shader/@renderconf.toLowerCase()@/@program@.h>
template<>
rhfw::render::ShaderProgram* rhfw::instantiateShader<rhfw::RenderConfig::@renderconf@, rhfw::UnifiedShaders::@program@>(rhfw::render::Renderer* param){
	return new rhfw::@renderconf@@program@{ static_cast<rhfw::@renderconf@Renderer*>(param) };
}
	@
@

@foreach renderconf in rendererconfigs:
	@foreach shader in shaderclassnames:
#include <gen/shader/@renderconf.toLowerCase()@/@shader@.h>
template<>
rhfw::render::ShaderPipelineStage* rhfw::instantiatePipeline<rhfw::RenderConfig::@renderconf@, rhfw::UnifiedShaderPipelineStage::@shader@>(rhfw::render::Renderer* param){
	return new rhfw::@renderconf@@shader@{ static_cast<rhfw::@renderconf@Renderer*>(param) };
}
	@
@

namespace rhfw {
static render::ShaderProgram* (*shaderLUT[(unsigned int) RenderConfig::_count_of_entries][(unsigned int) UnifiedShaders::_count_of_entries])(
		render::Renderer*) = {
			@foreach renderconf in rendererconfigs:{
				@foreach program in programnames:
				instantiateShader<RenderConfig::@renderconf@, UnifiedShaders::@program@>,@
			},@
		};
static render::ShaderPipelineStage* (*pipelinesLUT[(unsigned int) RenderConfig::_count_of_entries][(unsigned int) UnifiedShaderPipelineStage::_count_of_entries])(
		render::Renderer*) = {
			@foreach renderconf in rendererconfigs:{
				@foreach shader in shaderclassnames:
				instantiatePipeline<RenderConfig::@renderconf@, UnifiedShaderPipelineStage::@shader@>,@
			},@
		};

render::ShaderProgram* instantiateShader(UnifiedShaders shader, render::Renderer* rendererparam) {
	ASSERT(rendererparam->getRendererType() < RenderConfig::_count_of_entries) << "Renderer is out of bounds";
	ASSERT(shader < UnifiedShaders::_count_of_entries) << "Shader is out of bounds";
	return shaderLUT[(unsigned int) rendererparam->getRendererType()][(unsigned int) shader](rendererparam);
}
render::ShaderPipelineStage* instantiatePipeline(UnifiedShaderPipelineStage pipeline, render::Renderer* rendererparam) {
	ASSERT(rendererparam->getRendererType() < RenderConfig::_count_of_entries) << "Renderer is out of bounds";
	ASSERT(pipeline < UnifiedShaderPipelineStage::_count_of_entries) << "Pipeline is out of bounds";
	return pipelinesLUT[(unsigned int) rendererparam->getRendererType()][(unsigned int) pipeline](rendererparam);
}

}  // namespace rhfw
