#ifndef GEN_RENDERERS_H_
#define GEN_RENDERERS_H_

#include <gen/fwd/types.h>

namespace rhfw {
namespace render {

class Renderer;
class RenderingContext;
class RenderingContextOptions;

}  // namespace Renderer

@foreach renderapi in this :
#define RENDERAPI_@renderapi@_AVAILABLE 1
@

template<RenderConfig Renderer>
render::Renderer* instantiateRenderer();
template<RenderConfig Renderer>
render::RenderingContext* instantiateRenderingContext(render::Renderer* renderer, const render::RenderingContextOptions* options = nullptr);

render::RenderingContext* instantiateRenderingContext(render::Renderer* renderer, const render::RenderingContextOptions* options = nullptr);
render::Renderer* instantiateRenderer(RenderConfig renderer);

}  // namespace rhfw

#endif /* GEN_RENDERERS_H_ */
