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
#ifndef DIRECTXRENDERBUFFER_H_
#define DIRECTXRENDERBUFFER_H_

#include <framework/render/RenderBuffer.h>
#include <directx11/DirectX11Renderer.h>

namespace rhfw {

class DirectX11RenderBuffer: public render::RenderBuffer {
private:
	DirectX11Renderer* renderer;

	ID3D11Texture2D* texture = nullptr;
protected:
	virtual bool load() override {
		D3D11_TEXTURE2D_DESC depthStencilDesc;

		depthStencilDesc.Width = getWidth();
		depthStencilDesc.Height = getHeight();
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D16_UNORM;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		ThrowIfFailed(renderer->dev->CreateTexture2D(&depthStencilDesc, nullptr, &texture));
		return true;
	}
	virtual void free() override {
		texture->Release();
		texture = nullptr;
	}
	virtual bool reload() override {
		texture->Release();
		return load();
	}

public:
	DirectX11RenderBuffer(DirectX11Renderer* renderer)
			: renderer { renderer } {
	}

	DirectX11Renderer* getRenderer() {
		return renderer;
	}
	ID3D11Texture2D* getTexture() {
		return texture;
	}
};

} // namespace rhfw

#endif // DIRECTXRENDERBUFFER_H_

