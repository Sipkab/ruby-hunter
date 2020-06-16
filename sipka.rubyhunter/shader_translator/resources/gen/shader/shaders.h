#ifndef GEN_SHADERS_SHADERS_H_
#define GEN_SHADERS_SHADERS_H_

#include <gen/fwd/types.h>

namespace rhfw {
namespace render {
class ShaderProgram;
class Renderer;
class ShaderPipelineStage;
}  // namespace render
}  // namespace rhfw

namespace rhfw {
render::ShaderProgram* instantiateShader(UnifiedShaders shader, render::Renderer* rendererparam);
render::ShaderPipelineStage* instantiatePipeline(UnifiedShaderPipelineStage pipeline, render::Renderer* rendererparam);
}  // namespace rhfw


#endif /* GEN_SHADERS_SHADERS_H_ */
