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
#include <winstoreplatform/WinstoreDirectX11RenderingContext.h>
#include <directx11/DirectX11Renderer.h>

#include <framework/core/Window.h>

#include <winstoreplatform/WinstorePlatform.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/renderers.h>

#include WINDOWS_EXECPTION_HELPER_INCLUDE

#pragma comment( lib, "d3d11.lib")

namespace rhfw {
namespace render {

class WinstoreDirectX11RendererSurface final : public core::WindowSizeListener,
		public render::RenderTarget,
		private windowsstore::ApplicationStateListener {
private:
	WinstoreDirectX11RenderingContext* context;
	core::Window* window;

	IDXGISwapChain1* swapchain = nullptr;
	ID3D11RenderTargetView* rendertarget = nullptr;

	ID3D11DepthStencilView* depthStencilView = nullptr;

	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
	D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };
protected:
	virtual bool load() override {
		scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;              // the most common swap chain format
		scd.Stereo = false;
		scd.SampleDesc.Count = 1;                             // disable anti-aliasing
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
		scd.BufferCount = 2;                                  // a front buffer and a back buffer
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;    // the recommended flip mode
		scd.Flags = 0;
		scd.Scaling = DXGI_SCALING_NONE;
		scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		window->sizeListeners += *this;

		windowsstore::ApplicationStateListener::subscribe();

		auto wsize = window->getWindowSize();
		if (wsize.pixelSize.width() != 0 && wsize.pixelSize.height() != 0) {
			onSizeChanged(*window, wsize);
		}
		return true;
	}
	virtual void free() override {
		ULONG res;
		res = swapchain->Release();
		ASSERT(res == 0);
		res = rendertarget->Release();
		ASSERT(res == 0);
		res = depthStencilView->Release();
		ASSERT(res == 0);

		windowsstore::ApplicationStateListener::unsubscribe();

		window->sizeListeners -= *this;
	}

	virtual void applicationSuspending() {
		IDXGIDevice3* dxgiDevice;
		ThrowIfFailed(context->dev->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
		dxgiDevice->Trim();
		dxgiDevice->Release();
	}
	virtual void applicationResuming() {
	}

	void createSwapChain() {
		// obtain the DXGI factory
		IDXGIDevice* dxgiDevice;
		ThrowIfFailed(context->dev->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));
		IDXGIAdapter* dxgiAdapter;
		ThrowIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));
		dxgiDevice->Release();
		IDXGIFactory2* dxgiFactory;
		ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));
		dxgiAdapter->Release();

		//this gives 
		//DXGI WARNING: IDXGIFactory::CreateSwapChain:
		//	DXGI_SWAP_CHAIN_DESC.OutputWindow is not a valid window handle. [ MISCELLANEOUS WARNING #65: ]
		//on mobile emulator
		// create the swap chain
		ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow( context->dev,// address of the device
				reinterpret_cast<IUnknown*>(window->getNativeWindow()),// address of the window
				&scd,// address of the swap chain description
				nullptr,// advanced
				&swapchain)// address of the new swap chain pointer
				);
		dxgiFactory->Release();
	}
	void resizeSwapChain() {
		if (context->getRenderer()->currentRenderTarget == rendertarget) {
			ID3D11RenderTargetView* ar[1] { nullptr };
			context->devcon->OMSetRenderTargets(1, ar, nullptr);
		}
		rendertarget->Release();
		ThrowIfFailed(swapchain->ResizeBuffers(2, scd.Width, scd.Height, scd.Format, scd.Flags));
	}
public:

	WinstoreDirectX11RendererSurface(WinstoreDirectX11RenderingContext* context, core::Window* window)
			: context { context }, window { window } {
		load();
	}
	~WinstoreDirectX11RendererSurface() {
		free();
	}

	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override {
		depthStencilDesc.Width = size.pixelSize.width();
		depthStencilDesc.Height = size.pixelSize.height();
		scd.Width = depthStencilDesc.Width;
		scd.Height = depthStencilDesc.Height;

		if (swapchain == nullptr) {
			createSwapChain();
		} else {
			resizeSwapChain();
		}

		// get a pointer directly to the back buffer
		ID3D11Texture2D* backbuffer;
		ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));

		// create a render target pointing to the back buffer
		ThrowIfFailed(context->dev->CreateRenderTargetView(backbuffer, nullptr, &rendertarget));
		backbuffer->Release();

		if (depthStencilView != nullptr) {
			depthStencilView->Release();
		}
		ID3D11Texture2D* depthStencilBuffer;
		ThrowIfFailed(context->dev->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer));
		ThrowIfFailed(context->dev->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView));
		depthStencilBuffer->Release();
	}

	virtual void bindForDrawing() override {
		context->getRenderer()->currentRenderTarget = rendertarget;
		context->getRenderer()->currentDepthStencilView = depthStencilView;
		context->devcon->OMSetRenderTargets(1, &rendertarget, depthStencilView);
	}
	virtual void finishDrawing() override {
		ThrowIfFailed(swapchain->Present((UINT ) context->swapInterval, 0));
	}
	virtual render::ViewPort getDefaultViewPort() override {
		return render::ViewPort { 0, 0, scd.Width, scd.Height };
	}
};

WinstoreDirectX11RenderingContext::WinstoreDirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev,
		ID3D11DeviceContext1* devcon, Renderer* renderer)
		: DirectX11RenderingContext(functions, dev, devcon), renderer { static_cast<DirectX11Renderer*>(renderer) } {
}
void WinstoreDirectX11RenderingContext::attachWindow(core::Window * window) {
	ASSERT(window != nullptr) << "Window to attach is nullptr";
	ASSERT(!window->getRenderSurface().isAttached()) << "Window is already attached";
	Resource<render::WinstoreDirectX11RendererSurface> surface { new ResourceBlock { new WinstoreDirectX11RendererSurface { this, window } } };

	window->getRenderSurface().set(renderer, surface);
}

void WinstoreDirectX11RenderingContext::detachWindow(core::Window * window) {
	window->getRenderSurface().reset();
}

VSyncOptions WinstoreDirectX11RenderingContext::getSupportedVSyncOptions() {
	return VSyncOptions::VSYNC_ON | VSyncOptions::VSYNC_OFF | VSyncOptions::SWAPINTERVAL;
}

RenderingContextOptionsResult WinstoreDirectX11RenderingContext::setVSyncOptions(VSyncOptions options) {
	switch (options & VSyncOptions::TYPE_MASK) {
		case VSyncOptions::VSYNC_OFF:
			this->swapInterval = 0;
			break;
		case VSyncOptions::VSYNC_ON:
			this->swapInterval = 1;
			break;
		case VSyncOptions::SWAPINTERVAL:
			this->swapInterval = (int) ((options & VSyncOptions::SWAPINTERVAL_MASK) >> VSyncOptions::SWAPINTERVAL_SHIFT);
			break;
		default: {
			LOGW()<<"Unsupported VSync option: " << options;
			return RenderingContextOptionsResult::FAILED;
		}
	}
	this->contextOptions.vsyncOptions = options;
	return RenderingContextOptionsResult::SUCCESS;
}

} // namespace render

template<>
render::RenderingContext* instantiateRenderingContext<RenderConfig::DirectX11>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	render::DirectX11Functions dxfunc;
	dxfunc.dxfunc_D3D11CreateDevice = D3D11CreateDevice;

	ID3D11Device* dev11 = nullptr;
	ID3D11DeviceContext* devcon11 = nullptr;
	ID3D11Device1* dev = nullptr;
	ID3D11DeviceContext1* devcon = nullptr;
	UINT creationFlags = 0;
#if RHFW_DEBUG
	if (USE_DX11_DEBUG_CREATION_FLAG()) {
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif
	HRESULT hres = dxfunc.dxfunc_D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, nullptr, 0, D3D11_SDK_VERSION,
			&dev11, nullptr, &devcon11);
	if (hres != S_OK) {
		THROW()<< "Failed to create DirectX11 device " << hres;
		return nullptr;
	}
	ThrowIfFailed(dev11->QueryInterface(IID_PPV_ARGS(&dev)));
	ThrowIfFailed(devcon11->QueryInterface(IID_PPV_ARGS(&devcon)));
	devcon11->Release();
	dev11->Release();

	return new render::WinstoreDirectX11RenderingContext { dxfunc, dev, devcon, renderer };
}

} // namespace rhfw
