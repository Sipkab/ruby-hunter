#ifndef @shader_program_h_guard@
#define @shader_program_h_guard@

#include <gen/shader/@shader_program_h_classname@.h>
#include <gen/shader/opengles20/@shader_program_h_vertexshadertype@.h>
#include <gen/shader/opengles20/@shader_program_h_fragmentshadertype@.h>
#include <opengles20/shader/OpenGlEs20ShaderProgramBase.h>
#include <opengles20/buffer/OpenGlEs20VertexBuffer.h>

namespace rhfw {

	class OpenGlEs20@shader_program_h_classname @ final: public OpenGlEs20ShaderProgramBase<@shader_program_h_classname@> {
	private:
		class InputLayoutImpl final: public InputLayout {
		private:
			OpenGlEs20@shader_program_h_classname @* program;

		protected:
			virtual bool load() override {
				return true;
			}
			virtual void free() override {
			}
			virtual bool reload() override {
				free();
				return load();
			}
		public:
			InputLayoutImpl(OpenGlEs20@shader_program_h_classname @* program) : program {program} {}

			virtual void activate() override {
				@replace:shader_program_h_inputlayout_activate@
			}
		};
		@replace:shader_program_h_private_members@

	protected:
		virtual void onUseProgram() override {
			@replace:shader_program_h_program_onuseprogram@
		}

		virtual bool load() override {
			bool res = executeLoad<OpenGlEs20@shader_program_h_vertexshadertype@, OpenGlEs20@shader_program_h_fragmentshadertype@>();
			@replace:shader_program_h_program_created@
			return res;
		}
	public:
		OpenGlEs20@shader_program_h_classname@(OpenGlEs20Renderer* renderer) : OpenGlEs20ShaderProgramBase<@shader_program_h_classname@> {renderer} {
		}

		@replace:shader_program_h_public_members@

		virtual InputLayoutImpl* createInputLayoutImpl() override {
			return new InputLayoutImpl {this};
		}
	};

}  // namespace rhfw

#endif
