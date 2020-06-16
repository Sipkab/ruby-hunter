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
#ifndef DIRECTXRENDERTARGET_H_
#define DIRECTXRENDERTARGET_H_

#include <framework/render/RenderTarget.h>
#include <directx11/DirectX11Renderer.h>
#include <directx11/texture/DirectX11Texture.h>
#include <directx11/DirectX11RenderBuffer.h>

namespace rhfw {

class DirectX11RenderTarget: public render::RenderTarget {
private:
	DirectX11Renderer* renderer;

	ID3D11RenderTargetView* renderTargetView = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;

	void setColorAttachment(RenderTargetType type, Resource<RenderObject> obj) {
		switch (type) {
			case RenderTargetType::TEXTURE: {
				ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
				auto& texture = static_cast<DirectX11Texture&>(*obj);

				ThrowIfFailed(renderer->dev->CreateRenderTargetView(texture.getTextureName(), nullptr, &renderTargetView));
				ASSERT(renderTargetView != nullptr);
				break;
			}
			case RenderTargetType::RENDERBUFFER: {
				ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
				auto& buffer = static_cast<DirectX11RenderBuffer&>(*obj);

				ThrowIfFailed(renderer->dev->CreateRenderTargetView(buffer.getTexture(), nullptr, &renderTargetView));
				ASSERT(renderTargetView != nullptr);
				break;
			}
			case RenderTargetType::UNUSED: {
				//stay nullptr
				break;
			}
			default: {
				THROW()<<"Unsupported render target type: " << type;
				break;
			}
		}
	}
	void setDepthStencilAttachment(RenderTargetType type, Resource<RenderObject> obj) {
		switch (type) {
			case RenderTargetType::TEXTURE: {
				ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
				auto& texture = static_cast<DirectX11Texture&>(*obj);

				ThrowIfFailed(
						renderer->dev->CreateDepthStencilView(texture.getTextureName(), nullptr, &depthStencilView)
				);
				ASSERT(depthStencilView != nullptr);
				break;
			}
			case RenderTargetType::RENDERBUFFER: {
				ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
				auto& buffer = static_cast<DirectX11RenderBuffer&>(*obj);

				ThrowIfFailed(
						renderer->dev->CreateDepthStencilView(buffer.getTexture(), nullptr, &depthStencilView)
				);
				ASSERT(depthStencilView != nullptr);
				break;
			}
			case RenderTargetType::UNUSED: {
				//stay nullptr
				break;
			}
			default: {
				THROW() << "Unsupported render target type: " << type;
				break;
			}
		}
	}
protected:

	virtual bool load() override {
		const render::RenderTargetDescriptor& desc = getDescriptor();

		setColorAttachment(desc.getColorType(), desc.getColorTarget());
		setDepthStencilAttachment(desc.getDepthStencilType(), desc.getDepthStencilTarget());

		return true;
	}
	virtual void free() override {
		if (renderTargetView != nullptr) {
			renderTargetView->Release();
			renderTargetView = nullptr;
		}
		if (depthStencilView != nullptr) {
			depthStencilView->Release();
			depthStencilView = nullptr;
		}
	}
	virtual bool reload() override {
		free();
		return load();
	}
public:
	DirectX11RenderTarget(DirectX11Renderer* renderer) : renderer {renderer} {
	}

	virtual void bindForDrawing() override {
		renderer->devcon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
		renderer->currentRenderTarget = renderTargetView;
		renderer->currentDepthStencilView = depthStencilView;
	}
	virtual void finishDrawing() override {
		//TODO if ours are nullptr, then do not reset
		if (renderer->currentRenderTarget == renderTargetView || renderer->currentDepthStencilView == depthStencilView) {
			ID3D11RenderTargetView* ar[1] {nullptr};
			renderer->devcon->OMSetRenderTargets(1, ar, nullptr);
			renderer->currentRenderTarget = nullptr;
			renderer->currentDepthStencilView = nullptr;
		}
	}
	virtual render::ViewPort getDefaultViewPort() override {
		//TODO really should redo this
		render::RenderObject* ro = getDescriptor().getColorTarget().operator ->();
		render::SizedObject* obj =
		getDescriptor().getColorType() == RenderTargetType::TEXTURE ?
		static_cast<render::SizedObject*>(static_cast<render::Texture*>(ro)) :
		static_cast<render::SizedObject*>(static_cast<render::RenderBuffer*>(ro));
		return render::ViewPort {0, 0, obj->getWidth(), obj->getHeight()};
	}

	DirectX11Renderer* getRenderer() {
		return renderer;
	}
};

}
 // namespace rhfw

#endif // DIRECTXRENDERTARGET_H_
