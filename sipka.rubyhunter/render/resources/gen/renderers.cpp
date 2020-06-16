#include <gen/renderers.h>
#include <gen/log.h>
#include <gen/types.h>
#include <framework/render/Renderer.h>

namespace rhfw {

static render::Renderer* (*rendererLUT[(unsigned int) RenderConfig::_count_of_entries])() =
{
@foreach item in this :
	instantiateRenderer<RenderConfig::@item@>,
@
};

static render::RenderingContext* (*renderingContextLUT[(unsigned int) RenderConfig::_count_of_entries])(render::Renderer*, const render::RenderingContextOptions*) =
{
@foreach item in this :
	instantiateRenderingContext<RenderConfig::@item@>,
@
};

render::Renderer* instantiateRenderer(RenderConfig renderer) {
	ASSERT(renderer < RenderConfig::_count_of_entries) << "Renderer is out of bounds";
	return rendererLUT[(unsigned int) renderer]();
}
render::RenderingContext* instantiateRenderingContext(render::Renderer* renderer, const render::RenderingContextOptions* options) {
	ASSERT(renderer->getRendererType() < RenderConfig::_count_of_entries) << "Renderer is out of bounds";
	return renderingContextLUT[(unsigned int) renderer->getRendererType()](renderer, options);
}

}  // namespace rhfw
