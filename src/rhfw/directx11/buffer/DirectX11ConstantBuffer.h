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
 * DirectX11ConstantBuffer.h
 *
 *  Created on: 2015 febr. 6
 *      Author: sipka
 */

#ifndef DIRECTXCONSTANTBUFFER_H_
#define DIRECTXCONSTANTBUFFER_H_

#include <directx11/DirectX11Renderer.h>

#include <gen/configuration.h>
#include <gen/log.h>

#include <d3d11.h>

#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {

class DirectX11ConstantBuffer final : public ShareableResource {
private:
	DirectX11Renderer* renderer;
	const unsigned int size;

	ID3D11Buffer* buffer = nullptr;
public:

	virtual bool load() override {
		//TODO default usage helyett dynamic, �s Map() Unmap()
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = size;
		//desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		//desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		ThrowIfFailed(renderer->dev->CreateBuffer(&desc, nullptr, &buffer));
		return true;
	}
	virtual void free() override {
		buffer->Release();
	}

	DirectX11ConstantBuffer(DirectX11Renderer* renderer, unsigned int bytecount)
			: renderer { renderer }, size { bytecount } {
		ASSERT(bytecount > 0) << "cant create buffer with size 0";
	}
	DirectX11ConstantBuffer(const DirectX11ConstantBuffer&) = delete;
	DirectX11ConstantBuffer& operator=(const DirectX11ConstantBuffer&) = delete;
	DirectX11ConstantBuffer(DirectX11ConstantBuffer&&) = delete;
	DirectX11ConstantBuffer& operator=(DirectX11ConstantBuffer&&) = delete;
	~DirectX11ConstantBuffer() = default;

	void update(const void* data) {
		ASSERT(buffer != nullptr) << "Constant buffer is not loaded";
		ASSERT(data != nullptr) << "buffer data is nullptr";

		renderer->devcon->UpdateSubresource(buffer, 0, nullptr, data, 0, 0);
	}

	ID3D11Buffer* getDirectX11Name() const {
		return buffer;
	}
	DirectX11Renderer* getRenderer() {
		return renderer;
	}
};

}

#endif /* DIRECTXCONSTANTBUFFER_H_ */
