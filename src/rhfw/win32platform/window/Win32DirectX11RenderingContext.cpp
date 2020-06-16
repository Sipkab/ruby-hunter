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
#undef NOGDI
#include <windows.h>
#include <win32platform/window/Win32DirectX11RenderingContext.h>
#include <win32platform/LibraryHandle.h>
#include <directx11/DirectX11Renderer.h>

#include <framework/core/Window.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/renderers.h>

#include <math.h>

#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {
namespace render {
class Win32DirectX11RenderingContext;

class Win32DirectX11RendererSurface final : public core::WindowSizeListener, public RenderTarget, private core::WindowAccesStateListener {
private:
	friend Win32DirectX11RenderingContext;

	Win32DirectX11RenderingContext* context;
	core::Window* window;
	HWND hwnd;

	IDXGISwapChain1* swapchain = nullptr;
	ID3D11RenderTargetView* rendertarget = nullptr;

	ID3D11DepthStencilView* depthStencilView = nullptr;

	DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
	D3D11_TEXTURE2D_DESC depthStencilDesc = { 0 };

	bool shouldBeExclusiveFullScreen = false;
	bool exclusiveFullScreen = false;
	Size2UI fullScreenSize { 0, 0 };

	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) {
		if (swapchain == nullptr) {
			return;
		}
		if (inputFocused) {
			if (shouldBeExclusiveFullScreen && !exclusiveFullScreen) {
				executeFullScreenChange(true);
			}
		} else {
			if (shouldBeExclusiveFullScreen && exclusiveFullScreen) {
				executeFullScreenChange(false);
//				window.show(SW_MINIMIZE);
			}
		}
	}
protected:
	virtual bool load() override {
		scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scd.Stereo = false;
		scd.SampleDesc.Count = context->getRenderingContextOptions().multiSamplingFactor;
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = 2;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		//scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scd.Scaling = DXGI_SCALING_STRETCH;
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		//ThrowIfFailed(context->dev->CheckMultisampleQualityLevels(scd.Format, scd.SampleDesc.Count, &scd.SampleDesc.Quality));
		//scd.SampleDesc.Quality--;

		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilDesc.SampleDesc.Count = scd.SampleDesc.Count;
		depthStencilDesc.SampleDesc.Quality = scd.SampleDesc.Quality;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		window->sizeListeners += *this;
		window->accesStateListeners += *this;

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
	}

	bool getFullscreenDisplayMode(DXGI_MODE_DESC* out, IDXGIOutput* dxgiout) {
		DXGI_MODE_DESC* found = nullptr;

		UINT num = 0;
		DXGI_FORMAT format = scd.Format;
		UINT flags = DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING;

		dxgiout->GetDisplayModeList(format, flags, &num, nullptr);
		if (num == 0) {
			return false;
		}

		DXGI_MODE_DESC* pDescs = new DXGI_MODE_DESC[num];
		ThrowIfFailed(dxgiout->GetDisplayModeList(format, flags, &num, pDescs));
		int maxres = 0;
		float refresh = 0;
		for (UINT i = 0; i < num; ++i) {
			int res = pDescs[i].Width * pDescs[i].Height;
			float rate = pow(pDescs[i].RefreshRate.Numerator, pDescs[i].RefreshRate.Denominator);
			if (res > maxres) {
				maxres = res;
				refresh = rate;
				found = pDescs + i;
			} else if (res == maxres && rate > refresh) {
				refresh = rate;
				found = pDescs + i;
			}
		}
		*out = *found;
		delete[] pDescs;
		return true;
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

		// create the swap chain
		ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(context->dev, hwnd, &scd, nullptr, nullptr, &swapchain));
		ThrowIfFailed(dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES));
		dxgiFactory->Release();

		exclusiveFullScreen = false;
		if (shouldBeExclusiveFullScreen) {
			IDXGIOutput* dxgiout;
			ThrowIfFailed(swapchain->GetContainingOutput(&dxgiout));

			DXGI_MODE_DESC dxgimodedesc;
			if (!getFullscreenDisplayMode(&dxgimodedesc, dxgiout)) {
				dxgiout->Release();
				return;
			}

			fullScreenSize = {dxgimodedesc.Width, dxgimodedesc.Height};
			LOGI()<< "Full screen resolution: " << fullScreenSize;

			scd.Width = depthStencilDesc.Width = fullScreenSize.width();
			scd.Height = depthStencilDesc.Height = fullScreenSize.height();

			ThrowIfFailed(swapchain->ResizeBuffers(0, scd.Width, scd.Height, DXGI_FORMAT_UNKNOWN, scd.Flags));
			HRESULT sfsres = swapchain->SetFullscreenState(TRUE, dxgiout);
			dxgiout->Release();

			ASSERT(!FAILED(sfsres)) << sfsres;
			if (FAILED(sfsres)) {
				return;
			}

			ThrowIfFailed(swapchain->ResizeTarget(&dxgimodedesc));

			if (scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL || scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) {
				//just ResizeBuffers, do not care about rendertarget now
				//call only to SetFullScreenState to take effect
				//only call it when we are using FLIP presentation model according to documentation.
				ThrowIfFailed(swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, scd.Flags));
			}
			exclusiveFullScreen = true;
		}
	}
	void resizeSwapChain() {
		if (context->getRenderer()->currentRenderTarget == rendertarget) {
			ID3D11RenderTargetView* ar[1] { nullptr };
			context->devcon->OMSetRenderTargets(1, ar, nullptr);
		}
		rendertarget->Release();
		//keep buffer count and format the same
		ThrowIfFailed(swapchain->ResizeBuffers(0, scd.Width, scd.Height, DXGI_FORMAT_UNKNOWN, scd.Flags));
	}
	void recreateStencilViews() {
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
public:

	Win32DirectX11RendererSurface(Win32DirectX11RenderingContext* context, core::Window* window)
			: context { context }, window { window }, hwnd { window->getHwnd() } {
		load();
	}
	~Win32DirectX11RendererSurface() {
		free();
	}

	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override {
		if (scd.Width == size.pixelSize.width() && scd.Height == size.pixelSize.height()) {
			return;
		}
		scd.Width = depthStencilDesc.Width = size.pixelSize.width();
		scd.Height = depthStencilDesc.Height = size.pixelSize.height();

		if (swapchain == nullptr) {
			createSwapChain();
		} else {
			if (exclusiveFullScreen) {
				BOOL fullscreen;
				ThrowIfFailed(swapchain->GetFullscreenState(&fullscreen, NULL));
				if (!fullscreen) {
					exclusiveFullScreen = false;
				} else {
					return;
				}
			}
			//we could use ResizeBuffers on the swapchain,
			//however when rapidly resizing the window, it could generate E_OUTOFMEMORY
			//therefore, we are recreating it.
			//this only happens, when we are using FLIP swap model
			//we still might not need to use this, if we use the DXGI_MWA_NO_WINDOW_CHANGES flag in MakeWindowAssociation
//			if (/*scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL && */!shouldBeExclusiveFullScreen) {
//				if (context->getRenderer()->currentRenderTarget == rendertarget) {
//					ID3D11RenderTargetView* ar[1] { nullptr };
//					context->devcon->OMSetRenderTargets(1, ar, nullptr);
//				}
//				rendertarget->Release();
//				swapchain->Release();
//				createSwapChain();
//			} else {
//			}
			resizeSwapChain();
		}

		recreateStencilViews();
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

	void multiSamplingChanged(unsigned int count) {
		if (count == scd.SampleDesc.Count) {
			return;
		}
		if (swapchain == nullptr) {
			depthStencilDesc.SampleDesc.Count = scd.SampleDesc.Count = count;
			return;
		}

		IDXGIOutput* dxgiout = nullptr;
		BOOL fullscreen;
		ThrowIfFailed(swapchain->GetFullscreenState(&fullscreen, &dxgiout));
		if (fullscreen) {
			ThrowIfFailed(swapchain->SetFullscreenState(FALSE, NULL));
			resizeSwapChain();
			recreateStencilViews();
		}

		depthStencilDesc.SampleDesc.Count = scd.SampleDesc.Count = count;

		if (context->getRenderer()->currentRenderTarget == rendertarget) {
			ID3D11RenderTargetView* ar[1] { nullptr };
			context->devcon->OMSetRenderTargets(1, ar, nullptr);
		}
		rendertarget->Release();
		swapchain->Release();
		createSwapChain();

		if (dxgiout != nullptr) {
			ThrowIfFailed(swapchain->SetFullscreenState(TRUE, dxgiout));
			if (scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL || scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) {
				ThrowIfFailed(swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, scd.Flags));
			}
			dxgiout->Release();
		}
		recreateStencilViews();
	}

	bool executeFullScreenChange(bool fullscreen) {
		ASSERT(swapchain != nullptr);
		LOGI()<< "Executing DirectX full screen change: " << fullscreen;
		if (fullscreen) {
			IDXGIOutput* dxgiout;
			ThrowIfFailed(swapchain->GetContainingOutput(&dxgiout));

			DXGI_MODE_DESC dxgimodedesc;
			if (!getFullscreenDisplayMode(&dxgimodedesc, dxgiout)) {
				dxgiout->Release();
				exclusiveFullScreen = false;
				return false;
			}

			fullScreenSize = {dxgimodedesc.Width, dxgimodedesc.Height};
			LOGI()<< "Full screen resolution: " << fullScreenSize;

			scd.Width = depthStencilDesc.Width = fullScreenSize.width();
			scd.Height = depthStencilDesc.Height = fullScreenSize.height();

			//resize the buffers, so it will match the target resolution size,
			//so no mediator resolution would be set (which would distrupt the window positions on desktop)
			//not resizing, might result in throwin at ResizeBuffers after SetFullScreenState
			resizeSwapChain();

			HRESULT sfsres = swapchain->SetFullscreenState(TRUE, dxgiout);
			dxgiout->Release();

			if (sfsres == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
				BOOL fullscreen;
				ThrowIfFailed(swapchain->GetFullscreenState(&fullscreen, NULL));
				exclusiveFullScreen = fullscreen;

				recreateStencilViews();
				return exclusiveFullScreen;
			}
			ASSERT(!FAILED(sfsres)) << sfsres;

			ThrowIfFailed(swapchain->ResizeTarget(&dxgimodedesc));

			if (scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL || scd.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD) {
				//just ResizeBuffers, do not care about rendertarget now
				//call only to SetFullScreenState to take effect
				//only call it when we are using FLIP presentation model according to documentation.
				ThrowIfFailed(swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, scd.Flags));
			}
			exclusiveFullScreen = true;
		} else {
			ThrowIfFailed(swapchain->SetFullscreenState(FALSE, NULL));
			resizeSwapChain();
			exclusiveFullScreen = false;
		}
		recreateStencilViews();
		return exclusiveFullScreen;
	}

	bool isExclusiveFullScreen() const {
		return exclusiveFullScreen;
	}
	bool isShouldBeExclusiveFullScreen() const {
		return shouldBeExclusiveFullScreen;
	}
	bool setExclusiveFullScreen(bool fullscreen) {
		shouldBeExclusiveFullScreen = fullscreen;
		if (shouldBeExclusiveFullScreen == exclusiveFullScreen) {
			return exclusiveFullScreen;
		}
		if (swapchain == nullptr) {
			exclusiveFullScreen = false;
			return shouldBeExclusiveFullScreen;
		}

		return executeFullScreenChange(fullscreen);
	}
};

Win32DirectX11RenderingContext::Win32DirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev,
		ID3D11DeviceContext1* devcon, LibraryHandle libhandle, Renderer* renderer, const render::RenderingContextOptions* options)
		: DirectX11RenderingContext(functions, dev, devcon), libhandle(util::move(libhandle)), renderer {
				static_cast<DirectX11Renderer*>(renderer) } {
	if (options != nullptr) {
		this->contextOptions = *options;
	}
	if (this->contextOptions.multiSamplingFactor < 1) {
		this->contextOptions.multiSamplingFactor = 1;
	}
	setVSyncOptions(this->contextOptions.vsyncOptions);
	//TODO verify multisamplingfactor not greater than maximum in renderer
}
void Win32DirectX11RenderingContext::attachWindow(core::Window * window) {
	if (window->getLastRenderer() != RenderConfig::DirectX11 && window->getLastRenderer() != RenderConfig::_count_of_entries) {
		window->recreateWindow();
	} else {
		window->setLastRenderer(renderer->getRendererType());
		Resource<render::Win32DirectX11RendererSurface> surface { new ResourceBlock { new Win32DirectX11RendererSurface { this, window } } };
		window->getRenderSurface().set(renderer, surface);
	}
}

void Win32DirectX11RenderingContext::detachWindow(core::Window * window) {
	if (window->getRenderSurface().getRenderTarget() == nullptr) {
		return;
	}
	auto& surface = *(Win32DirectX11RendererSurface*) window->getRenderSurface().getRenderTarget();
	if (surface.isExclusiveFullScreen()) {
		ThrowIfFailed(surface.swapchain->SetFullscreenState(FALSE, NULL));
	}
	window->getRenderSurface().reset();
}

bool Win32DirectX11RenderingContext::supportsExclusiveFullScreen() {
	return true;
}
bool Win32DirectX11RenderingContext::setExclusiveFullScreen(core::Window* window, bool fullscreen) {
	ASSERT(window->getRenderSurface().getRenderer() == renderer);
	auto surface = static_cast<Win32DirectX11RendererSurface*>(window->getRenderSurface().getRenderTarget());
	return surface->setExclusiveFullScreen(fullscreen);
}
bool Win32DirectX11RenderingContext::isExclusiveFullScreen(core::Window* window) {
	ASSERT(window->getRenderSurface().getRenderer() == renderer);
	auto surface = (Win32DirectX11RendererSurface*) window->getRenderSurface().getRenderTarget();
	return surface->isExclusiveFullScreen();
}

VSyncOptions Win32DirectX11RenderingContext::getSupportedVSyncOptions() {
	return VSyncOptions::VSYNC_ON | VSyncOptions::VSYNC_OFF | VSyncOptions::SWAPINTERVAL;
}
RenderingContextOptionsResult Win32DirectX11RenderingContext::setVSyncOptions(VSyncOptions options) {
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

RenderingContextOptionsResult Win32DirectX11RenderingContext::setRenderingContextOptions(const RenderingContextOptions& options) {
	RenderingContextOptionsResult result = RenderingContextOptionsResult::NO_FLAG;
	UINT msfactor = options.multiSamplingFactor;
	if (msfactor > renderer->maxMultiSampleFactor) {
		msfactor = renderer->maxMultiSampleFactor;
	}
	if (msfactor < 1) {
		msfactor = 1;
	}
	if (msfactor != contextOptions.multiSamplingFactor) {
		contextOptions.multiSamplingFactor = msfactor;
		for (auto&& w : renderer->getAttachedWindows().objects()) {
			auto& surface = static_cast<Win32DirectX11RendererSurface&>(*w.getRenderSurface().getRenderTarget());
			surface.multiSamplingChanged(contextOptions.multiSamplingFactor);
		}
	}
	if (options.vsyncOptions != contextOptions.vsyncOptions) {
		result |= setVSyncOptions(options.vsyncOptions);
	}
	return result;
}

unsigned int Win32DirectX11RenderingContext::getMaximumMultiSampleFactor() {
	return renderer->maxMultiSampleFactor;
}

} // namespace render

static bool initDxFunctions(render::DirectX11Functions& func, HMODULE module) {
	func.dxfunc_D3D11CreateDevice = (render::DirectX11Functions::PROTO_D3D11CreateDevice) GetProcAddress(module, "D3D11CreateDevice");
	if (func.dxfunc_D3D11CreateDevice == nullptr) {
		LOGW()<< "Failed to load " << "D3D11CreateDevice " << GetLastError();
		return false;
	}
	return true;
}

template<>
render::RenderingContext* instantiateRenderingContext<RenderConfig::DirectX11>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	LibraryHandle libhandle { "D3D11.dll" };
	if (!libhandle) {
		LOGW()<< "Failed to load DirectX11 library: " << GetLastError();
		return nullptr;
	}
	render::DirectX11Functions dxfunc;
	if (!initDxFunctions(dxfunc, (HMODULE) libhandle)) {
		LOGW()<< "Failed to load DirectX functions";
		return nullptr;
	}
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

	return new render::Win32DirectX11RenderingContext { dxfunc, dev, devcon, util::move(libhandle), renderer, options };
}

} // namespace rhfw
