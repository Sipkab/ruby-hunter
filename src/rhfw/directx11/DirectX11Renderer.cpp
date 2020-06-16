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
#include <directx11/texture/DirectX11Texture.h>
#include <directx11/buffer/DirectX11IndexBuffer.h>
#include <directx11/buffer/DirectX11VertexBuffer.h>
#include <directx11/DirectX11RenderTarget.h>
#include <directx11/DirectX11RenderBuffer.h>
#include <directx11/DirectX11RenderingContext.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/VertexDataBaseChanger.h>
#include <framework/geometry/Vector.h>
#include <framework/geometry/Matrix.h>
#include <framework/resource/font/Font.h>

#include <gen/resources.h>
#include <gen/log.h>
#include <gen/renderers.h>
#include <gen/platform.h>
#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {

DirectX11Renderer::DirectX11Renderer()
		: Renderer { RenderConfig::DirectX11, TextureLoadOrder::FIRST_LINE_AT_ORIGIN, TextureUvOrientation::Y_DIR_DOWN,
				DepthRange::RANGE_0_TO_1 } {
}
DirectX11Renderer::~DirectX11Renderer() {
}

bool DirectX11Renderer::load() {
	if (!Renderer::load()) {
		return false;
	}

	dev = static_cast<render::DirectX11RenderingContext*>(getRenderingContext())->getDevice();
	devcon = static_cast<render::DirectX11RenderingContext*>(getRenderingContext())->getDeviceContext();
	D3D11_RASTERIZER_DESC wfdesc = { };
	//ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_SOLID;
	wfdesc.CullMode = D3D11_CULL_BACK;
	wfdesc.FrontCounterClockwise = TRUE;
	wfdesc.DepthClipEnable = TRUE;
	wfdesc.MultisampleEnable = TRUE;
	ThrowIfFailed(dev->CreateRasterizerState(&wfdesc, &rasterizerState));
	devcon->RSSetState(rasterizerState);
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	D3D11_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
	rtbd.BlendEnable = true;
	/*rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	 rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	 rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	 rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	 rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	 rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	 rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;*/
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.AlphaToCoverageEnable = false;

	ThrowIfFailed(dev->CreateBlendState(&blendDesc, &blendState));
	devcon->OMSetBlendState(blendState, nullptr, 0xffffffff);
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	dsDesc.StencilEnable = false;
	ThrowIfFailed(dev->CreateDepthStencilState(&dsDesc, &depthStencilStateWithDepthtest));
	dsDesc.DepthEnable = false;
	ThrowIfFailed(dev->CreateDepthStencilState(&dsDesc, &depthStencilStateWithoutDepthtest));
	devcon->OMSetDepthStencilState(depthStencilStateWithoutDepthtest, 0);
	IDXGIDevice1* dxgiDevice;
	ThrowIfFailed(dev->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
	IDXGIAdapter* dxgiAdapter;
	ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
	// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
	// ensures that the application will only render after each VSync, minimizing power consumption.
	ThrowIfFailed(dxgiDevice->SetMaximumFrameLatency(1));
	dxgiDevice->Release();

	{
		UINT outquality;
		UINT count = 4;
		//FEATURE_LEVEL_11_0 is required to support at least 4
		UINT test;
		while ((test = count * 2) <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT) {
			HRESULT res = dev->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, test, &outquality);
			if (res != S_OK || outquality == 0) {
				break;
			}
			LOGI()<< "MSAAx" << test << " Quality level: " << outquality;
			count = test;
		}
		maxMultiSampleFactor = count;
	}

	return true;
}
void DirectX11Renderer::free() {
	ULONG res;
	res = rasterizerState->Release();
	ASSERT(res == 0);
	res = blendState->Release();
	ASSERT(res == 0);
	res = depthStencilStateWithDepthtest->Release();
	ASSERT(res == 0);
	res = depthStencilStateWithoutDepthtest->Release();
	ASSERT(res == 0);

	//we might need these, to properly destroy HWND flip swap chains
	devcon->ClearState();
	devcon->Flush();

	Renderer::free();
}

void DirectX11Renderer::clearColor(const Color& color) {
	devcon->ClearRenderTargetView(currentRenderTarget, (const float*) color);
}
void DirectX11Renderer::clearDepthBuffer() {
	devcon->ClearDepthStencilView(currentDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
void DirectX11Renderer::clearColorDepthBuffer(const Color& color) {
	clearColor(color);
	clearDepthBuffer();
}
/*
 void DirectX11Renderer::activateRenderTarget(Texture* texture) {
 if (texture == 0) {
 // set our new render target object as the active render target
 devcon->OMSetRenderTargets(1, rendertarget.GetAddressOf(), depthStencilView.Get());
 currentRenderTarget = rendertarget.Get();
 currentDepthStencilView = depthStencilView.Get();
 return;
 }

 //TODO unbind-et ne igy
 ID3D11ShaderResourceView * n = nullptr;
 devcon->PSSetShaderResources(0, 1, &n);
 devcon->OMSetRenderTargets(1, texture->renderTargetView.GetAddressOf(), texture->depthStencilView.Get());

 currentRenderTarget = texture->renderTargetView.Get();
 currentDepthStencilView = texture->depthStencilView.Get();
 //TODO
 }
 */
void DirectX11Renderer::invalidateRenderTarget() {
	renderTargetState.postCommand();
}

render::Texture* DirectX11Renderer::createTextureImpl() {
	return new DirectX11Texture { this };
}
render::VertexBuffer* DirectX11Renderer::createVertexBufferImpl() {
	return new DirectX11VertexBuffer { this };
}
render::IndexBuffer* DirectX11Renderer::createIndexBufferImpl() {
	return new DirectX11IndexBuffer { this };
}
render::RenderTarget* DirectX11Renderer::createRenderTargetImpl() {
	return new DirectX11RenderTarget { this };
}
render::RenderBuffer* DirectX11Renderer::createRenderBufferImpl() {
	return new DirectX11RenderBuffer { this };
}

void DirectX11Renderer::setDepthTestImpl(bool enabled) {
	if (enabled) {
		devcon->OMSetDepthStencilState(depthStencilStateWithDepthtest, 0);
	} else {
		devcon->OMSetDepthStencilState(depthStencilStateWithoutDepthtest, 0);
	}
}
void DirectX11Renderer::setFaceCullingImpl(bool enabled) {
	if (enabled) {
	} else {
	}
}
void DirectX11Renderer::setCullToFrontFaceImpl(bool isFront) {
	if (isFront) {
	} else {
	}
}
void DirectX11Renderer::setViewPortImpl(const render::ViewPort& vp) {
	D3D11_VIEWPORT viewport = { 0 };

	viewport.TopLeftX = (float) vp.pos.x();
	viewport.TopLeftY = (float) vp.pos.y();
	viewport.Width = (float) vp.size.width();
	viewport.Height = (float) vp.size.height();
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;

	devcon->RSSetViewports(1, &viewport);
}

template<> render::Renderer* instantiateRenderer<RenderConfig::DirectX11>() {
	return new DirectX11Renderer { };
}

}
