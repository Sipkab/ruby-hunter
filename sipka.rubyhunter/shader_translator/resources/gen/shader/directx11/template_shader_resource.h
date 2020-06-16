#ifndef @shader_resource_h_guard@
#define @shader_resource_h_guard@

#include <directx11/DirectX11Renderer.h>
#include <directx11/DirectX11ShaderPipelineStage.h>
#include <directx11/buffer/DirectX11ConstantBuffer.h>
#include <directx11/texture/DirectX11Texture.h>
#include <directx11/HLSLMatrix.h>

#include <framework/resource/Resource.h>

#include <d3d11_2.h>
#include <framework/io/files/AssetFileDescriptor.h>
#include <fstream>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/assets.h>

namespace rhfw {

	class DirectX11@shader_resource_h_classname @ final: public DirectX11ShaderPipelineStage<ID3D11@shader_resource_h_comptr_type@, @shader_resource_h_classname@> {
	public:
		static const UnifiedShaderPipelineStage PIPELINE_STAGE = UnifiedShaderPipelineStage::@shader_resource_h_classname@;
	protected:
		bool load() override {
			AssetFileDescriptor fdesc {RAssets::@shader_resource_h_assetpath@};
			unsigned int len;
			shaderdata = fdesc.readFully(&len);
			shadersize = (int) len;

			//shaderdata = LoadShaderFile("@shader_resource_h_shaderfilename@", &shadersize);
			ASSERT(shaderdata != 0 && shadersize > 0) << "Failed to load shader @shader_resource_h_shaderfilename@";

			ThrowIfFailed(
					renderer->dev->Create@shader_resource_h_comptr_type@(shaderdata, shadersize, nullptr, &shader)
			);
			SET_DIRECTX_DEBUGNAME(shader, "DirectX11@shader_resource_h_classname @");

			return true;
		}
		void free() override {
			shader->Release();
			delete[] shaderdata;
		}
	public:
		@replace:shader_resource_h_public_members@

		DirectX11@shader_resource_h_classname@(DirectX11Renderer* renderer)
		: DirectX11ShaderPipelineStage<ID3D11@shader_resource_h_comptr_type@, @shader_resource_h_classname@>(renderer) {
		}
		~DirectX11@shader_resource_h_classname@() {
		}
	};
} // namespace rhfw

#endif
