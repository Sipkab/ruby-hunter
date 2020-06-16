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

namespace rhfw {
class DirectX11Renderer;
namespace render {
class WinstoreDirectX11RendererSurface;
class WinstoreDirectX11RenderingContext: public DirectX11RenderingContext {
private:
	friend WinstoreDirectX11RendererSurface;

	DirectX11Renderer* renderer;

	int swapInterval = 0;
public:
	WinstoreDirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev, ID3D11DeviceContext1* devcon,
			Renderer* renderer);
	~WinstoreDirectX11RenderingContext() {
		destroyContext();
	}

	virtual void attachWindow(core::Window* window) override;
	virtual void detachWindow(core::Window* window) override;

	DirectX11Renderer* getRenderer() {
		return renderer;
	}

	virtual VSyncOptions getSupportedVSyncOptions() override;
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) override;
};
} // namespace render
} // namespace rhfw

#endif /* DIRECTXRENDERINGCONTEXT_H_ */
