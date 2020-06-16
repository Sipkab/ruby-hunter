/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 */
#ifndef DIRECTXRENDERER_H_
#define DIRECTXRENDERER_H_

#include <framework/render/Renderer.h>

#include <gen/configuration.h>
#include <gen/platform.h>

#include <d3d11_2.h>

#if LOGGING_ENABLED
#pragma comment( lib, "dxguid.lib")
#define SET_DIRECTX_DEBUGNAME(obj, name) obj->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof( name ) - 1, name )
#else
#define SET_DIRECTX_DEBUGNAME(obj, name) 
#endif

namespace rhfw {

class DirectX11IndexBuffer;

class DirectX11Renderer final :public render::Renderer {
public:

	ID3D11Device1* dev = nullptr;
	ID3D11DeviceContext1* devcon = nullptr;

	ID3D11RasterizerState* rasterizerState = nullptr;
	ID3D11BlendState* blendState = nullptr;
	ID3D11DepthStencilState* depthStencilStateWithDepthtest = nullptr;
	ID3D11DepthStencilState* depthStencilStateWithoutDepthtest = nullptr;

	ID3D11RenderTargetView* currentRenderTarget = nullptr;
	ID3D11DepthStencilView* currentDepthStencilView = nullptr;

	DirectX11IndexBuffer* activeIndexBuffer = nullptr;

	Resource<render::ShaderPipelineStage> activePixelShader = nullptr;
	Resource<render::ShaderPipelineStage> activeVertexShader = nullptr;

	UINT maxMultiSampleFactor = 0;

private:
	virtual void setDepthTestImpl(bool enabled) override;
	virtual void setFaceCullingImpl(bool enabled) override;
	virtual void setCullToFrontFaceImpl(bool isFront) override;
	virtual void setViewPortImpl(const render::ViewPort& vp) override;

	virtual render::Texture* createTextureImpl() override;
	virtual render::VertexBuffer* createVertexBufferImpl() override;
	virtual render::IndexBuffer* createIndexBufferImpl() override;
	virtual render::RenderTarget* createRenderTargetImpl() override;
	virtual render::RenderBuffer* createRenderBufferImpl() override;
protected:

	virtual bool load() override;
	virtual void free() override;
public:

	DirectX11Renderer();
	virtual ~DirectX11Renderer();

	void invalidateRenderTarget();

	virtual void clearColor(const Color& color) override;
	virtual void clearDepthBuffer() override;
	virtual void clearColorDepthBuffer(const Color& color) override;

	virtual void setTopology(Topology topology) override {
		Renderer::setTopology(topology);
		switch (topology) {
			case Topology::TRIANGLES: {
				devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				break;
			}
			case Topology::TRIANGLE_STRIP: {
				devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				break;
			}
			default: {
				break;
			}
		}
	}
};

}

#endif /* DIRECTXRENDERER_H_ */
