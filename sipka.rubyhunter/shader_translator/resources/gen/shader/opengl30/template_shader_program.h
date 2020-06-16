#ifndef @shader_program_h_guard@
#define @shader_program_h_guard@

#include <gen/shader/@shader_program_h_classname@.h>
#include <gen/shader/opengl30/@shader_program_h_vertexshadertype@.h>
#include <gen/shader/opengl30/@shader_program_h_fragmentshadertype@.h>
#include <opengl30/shader/OpenGl30ShaderProgramBase.h>
#include <opengl30/buffer/OpenGl30VertexBuffer.h>

namespace rhfw {

	class OpenGl30@shader_program_h_classname @ final: public OpenGl30ShaderProgramBase<@shader_program_h_classname@> {
	private:
		class InputLayoutImpl final: public InputLayout {
		private:
			OpenGl30@shader_program_h_classname @* program;

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
			InputLayoutImpl(OpenGl30@shader_program_h_classname @* program) : program {program} {}

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
			bool res = executeLoad<OpenGl30@shader_program_h_vertexshadertype@, OpenGl30@shader_program_h_fragmentshadertype@>();
			@replace:shader_program_h_program_created@
			return res;
		}
	public:
		OpenGl30@shader_program_h_classname@(OpenGl30Renderer* renderer) : OpenGl30ShaderProgramBase<@shader_program_h_classname@>(renderer) {
		}

		@replace:shader_program_h_public_members@

		virtual InputLayoutImpl* createInputLayoutImpl() override {
			return new InputLayoutImpl {this};
		}
	};

}  // namespace rhfw

#endif
