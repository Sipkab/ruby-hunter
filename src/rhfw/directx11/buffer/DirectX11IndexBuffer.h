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
 * DirectX11Buffer.h
 *
 *  Created on: 2015 febr. 6
 *      Author: sipka
 */

#ifndef DIRECTXINDEXBUFFER_H_
#define DIRECTXINDEXBUFFER_H_

#include <directx11/DirectX11Renderer.h>
#include <framework/render/IndexBuffer.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {

class DirectX11IndexBuffer final : public render::IndexBuffer {
private:
	ID3D11Buffer* buffer = nullptr;
	DirectX11Renderer* renderer;

	virtual void allocateDataImpl(const void* data, unsigned int bytecount, BufferType bufferType) override {
		if (buffer != nullptr)
			buffer->Release();

		//TODO default usage helyett dynamic, ?s Map() Unmap(), lent is
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = bytecount;
		//desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subres;
		subres.pSysMem = data;
		subres.SysMemPitch = 0;
		subres.SysMemSlicePitch = 0;

		if (data != nullptr) {
			D3D11_SUBRESOURCE_DATA subres;
			subres.pSysMem = data;
			subres.SysMemPitch = 0;
			subres.SysMemSlicePitch = 0;

			ThrowIfFailed(renderer->dev->CreateBuffer(&desc, &subres, &buffer));
		} else {
			ThrowIfFailed(renderer->dev->CreateBuffer(&desc, nullptr, &buffer));
		}
		if (renderer->activeIndexBuffer == this) {
			renderer->devcon->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
		}
	}
	virtual void updateRegionImpl(const void* data, unsigned int start, unsigned int bytecount) override {
		ASSERT(buffer != nullptr) << "Buffer is not allocated";

		D3D11_BOX box;
		box.left = start;
		box.right = box.left + bytecount;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		renderer->devcon->UpdateSubresource(buffer, 0, &box, data, 0, 0);
	}
protected:
	virtual bool loadImpl() override {
		return true;
	}
	virtual void freeImpl() override {
		if (buffer != nullptr) {
			deactivate();
			buffer->Release();
			buffer = nullptr;
		}
	}
	virtual bool reloadImpl() override {
		return true;
	}
public:

	DirectX11IndexBuffer(DirectX11Renderer* renderer)
			: renderer { renderer } {
	}
	DirectX11IndexBuffer(const DirectX11IndexBuffer&) = delete;
	~DirectX11IndexBuffer() {
		ASSERT(buffer == nullptr) << "Buffer wasn't destroyed: " << buffer;
	}

	virtual void activate() override {
		ASSERT(buffer != nullptr) << "Buffer is not initialized";
		if (renderer->activeIndexBuffer != this) {
			renderer->devcon->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
			renderer->activeIndexBuffer = this;
		}
	}
	void deactivate() {
		if (renderer->activeIndexBuffer == this) {
			renderer->devcon->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
			renderer->activeIndexBuffer = this;
		}
	}

	ID3D11Buffer* getDirectX11Name() {
		return buffer;
	}
	DirectX11Renderer* getRenderer() {
		return renderer;
	}
};

}

#endif /* DIRECTXINDEXBUFFER_H_ */
