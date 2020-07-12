#ifndef @shader_program_h_guard@
#define @shader_program_h_guard@

#include <gen/shader/@shader_program_h_classname@.h>
#include <gen/shader/directx11/@shader_program_h_vertexshadertype@.h>
#include <gen/shader/directx11/@shader_program_h_fragmentshadertype@.h>
#include <directx11/buffer/DirectX11ConstantBuffer.h>
#include <directx11/buffer/DirectX11VertexBuffer.h>
#include <directx11/DirectX11ShaderProgramBase.h>
#include <directx11/texture/DirectX11Texture.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/assets.h>

namespace rhfw {

	class DirectX11@shader_program_h_classname @ final: public DirectX11ShaderProgramBase<@shader_program_h_classname@> {
	private:
		class InputLayoutImpl final: public InputLayout {
		private:
			DirectX11@shader_program_h_classname @* program;
			ID3D11InputLayout* d3d11layout = nullptr;

			const unsigned int offsets[@shader_program_base_inputlayout_member_count@] {0};
		protected:
			virtual bool load() override {
				D3D11_INPUT_ELEMENT_DESC IED[@shader_program_base_inputlayout_member_count@];
				@replace:shader_program_h_inputlayout_load@

				ThrowIfFailed(
						program->renderer->dev->CreateInputLayout(IED, @shader_program_base_inputlayout_member_count@,
								static_cast<DirectX11@shader_program_h_vertexshadertype@*>(program->vertexShader)->getShaderData(),
								static_cast<DirectX11@shader_program_h_vertexshadertype@*>(program->vertexShader)->getShaderDataSize(),
								&d3d11layout)
				);
				return true;
			}
			virtual void free() override {
				d3d11layout->Release();
				//TODO check if bound
			}
			virtual bool reload() override {
				free();
				return load();
			}
		public:
			InputLayoutImpl(DirectX11@shader_program_h_classname @* program) : program {program} {}

			virtual void activate() override {
				ID3D11Buffer* dxbufs[@shader_program_base_inputlayout_member_count@];
				for(unsigned int i = 0; i < bufferCount; ++i) {
					DirectX11VertexBuffer* buf = static_cast<DirectX11VertexBuffer*>(buffers[i]);
					dxbufs[i] = buf->getDirectX11Name();
				}
				program->renderer->devcon->IASetInputLayout(d3d11layout);
				program->renderer->devcon->IASetVertexBuffers(0, bufferCount, dxbufs, strides, offsets);
			}
		};

	protected:
		virtual bool load() override {
			bool res = executeLoad<DirectX11@shader_program_h_vertexshadertype@, DirectX11@shader_program_h_fragmentshadertype@>();
			return res;
		}
	public:
		@replace:shader_program_h_public_members@
		@foreach uniform in program.getUniforms() :
		virtual void set(@uniform.getName()@& u) override {
			static_cast<@translator.getUniqueName()@@uniform.getClassUrl().getExactClassName()@::@uniform.getName()@_impl&>(u).set();
		}
		@

		virtual void useProgram() override {
			if (renderer->activePixelShader != fragmentShader) {
				ID3D11PixelShader* pixel = static_cast<DirectX11@shader_program_h_fragmentshadertype@*>(fragmentShader)->getDirectX11Name();
				renderer->devcon->PSSetShader(pixel, nullptr, 0);
				renderer->activePixelShader = fragmentShader;
			}
			if (renderer->activeVertexShader != vertexShader) {
				ID3D11VertexShader* vertex = static_cast<DirectX11@shader_program_h_vertexshadertype@*>(vertexShader)->getDirectX11Name();
				renderer->devcon->VSSetShader(vertex, nullptr, 0);
				renderer->activeVertexShader = vertexShader;
			}
		}

		DirectX11@shader_program_h_classname@(DirectX11Renderer* renderer)
		: DirectX11ShaderProgramBase<@shader_program_h_classname@>(renderer) {
		}
		~DirectX11@shader_program_h_classname@() {
		}
		virtual InputLayoutImpl* createInputLayoutImpl() override {
			return new InputLayoutImpl {this};
		}
	};

} // namespace rhfw

#endif
