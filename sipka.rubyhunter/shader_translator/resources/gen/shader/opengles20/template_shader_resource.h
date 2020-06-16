#ifndef @shader_resource_h_guard@
#define @shader_resource_h_guard@

#include <opengles20/OpenGlEs20Renderer.h>
#include <opengles20/shader/OpenGlEs20ShaderPipelineStage.h>
#include <opengles20/texture/OpenGlEs20Texture.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/assets.h>

#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/io/files/FileInput.h>
#include <framework/resource/ResourceManager.h>
#include <gen/shader/@shader_resource_h_classname@.h>

@replace:shader_resource_h_includes @

namespace rhfw {

	class OpenGlEs20@shader_resource_h_classname @ final: public OpenGlEs20ShaderPipelineStage<@shader_resource_h_classname@> {
	public:
		static const UnifiedShaderPipelineStage PIPELINE_STAGE = UnifiedShaderPipelineStage::@shader_resource_h_classname@;
	private:
		@replace:shader_resource_h_private_members@

	protected:
		@replace:shader_resource_h_protected_members@
	protected:
		virtual bool load() override {
			shader = renderer->glCreateShader(@shader_resource_h_shadertype@);
			CHECK_GL_ERROR();
			ASSERT(shader != 0) << "Couldn't create shader";

			AssetFileDescriptor fdesc {RAssets::@shader_resource_h_assetfileid@};
			unsigned int len;
			const char* source = fdesc.readFully(&len);

			renderer->glShaderSource(shader, 1, &source, nullptr);
			CHECK_GL_ERROR();
			renderer->glCompileShader(shader);

			delete[] source;

#if RHFW_DEBUG
			GLint res;
			renderer->glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
			CHECK_GL_ERROR();

			if (res == GL_FALSE) {
				GLchar errordesc[512];
				GLsizei size;
				renderer->glGetShaderInfoLog(shader, sizeof errordesc, &size, errordesc);
				LOGWTF() << "Failed to compile shader: " << RAssets::@shader_resource_h_assetfileid@<< "\nError: " << errordesc;
				CHECK_GL_ERROR();
				renderer->glDeleteShader(shader);
				CHECK_GL_ERROR();
				THROW() << "Couldn't compile shader";
				return false;
			}
#endif
			return true;
		}
		virtual void free() override {
			renderer->glDeleteShader(shader);
			CHECK_GL_ERROR();
		}

	public:
		@replace:shader_resource_h_public_members@

		OpenGlEs20@shader_resource_h_classname@(OpenGlEs20Renderer* renderer) : OpenGlEs20ShaderPipelineStage {renderer} {
		}

		~OpenGlEs20@shader_resource_h_classname@() {
		}
	};

} // namespace rhfw

#endif
