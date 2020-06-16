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
#ifndef DIRECTXRENDERINGCONTEXT_H_
#define DIRECTXRENDERINGCONTEXT_H_

#include <directx11/DirectX11RenderingContext.h>
#include <win32platform/LibraryHandle.h>
#include <gen/types.h>

namespace rhfw {
class DirectX11Renderer;
namespace render {
class Win32DirectX11RendererSurface;

class Win32DirectX11RenderingContext: public DirectX11RenderingContext {
private:
	friend Win32DirectX11RendererSurface;

	LibraryHandle libhandle;
	DirectX11Renderer* renderer;

	int swapInterval = 0;

public:
	Win32DirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev, ID3D11DeviceContext1* devcon,
			LibraryHandle libhandle, Renderer* renderer, const render::RenderingContextOptions* options);
	~Win32DirectX11RenderingContext() {
		destroyContext();
	}

	virtual void attachWindow(core::Window* window) override;
	virtual void detachWindow(core::Window* window) override;

	DirectX11Renderer* getRenderer() {
		return renderer;
	}

	virtual bool supportsExclusiveFullScreen() override;
	virtual bool setExclusiveFullScreen(core::Window* window, bool fullscreen) override;
	virtual bool isExclusiveFullScreen(core::Window* window) override;

	virtual VSyncOptions getSupportedVSyncOptions() override;
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) override;

	virtual unsigned int getMaximumMultiSampleFactor() override;

	virtual RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) override;
};
} // namespace render
} // namespace rhfw

#endif /* DIRECTXRENDERINGCONTEXT_H_ */
