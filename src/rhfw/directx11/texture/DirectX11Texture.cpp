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
#include <directx11/DirectX11Renderer.h>
#include <directx11/texture/DirectX11Texture.h>

#include <gen/log.h>
#include <gen/types.h>

#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {
bool DirectX11Texture::createTextureResources() {
	auto* in = getInputSource();
	ASSERT(in != nullptr) << "Input source missing for texture";

	in->apply(this);
	if (textureResource == nullptr) {
		return false;
	}
	ThrowIfFailed(renderer->dev->CreateShaderResourceView(textureResource, nullptr, &shaderResource));
	return true;
}
bool DirectX11Texture::load() {
	if (!createTextureResources()) {
		return false;
	}

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ThrowIfFailed(renderer->dev->CreateSamplerState(&sampDesc, &samplerState));
	return true;
}
void DirectX11Texture::free() {
	samplerState->Release();
	textureResource->Release();
	shaderResource->Release();
}

bool DirectX11Texture::reload() {
	//samplerstate is good
	//textureResource->Release();
	//shaderResource->Release();

	//return createTextureResources();
	free();
	return load();
}

void DirectX11Texture::initAsEmpty(unsigned int width, unsigned int height, ColorFormat format) {
	D3D11_TEXTURE2D_DESC desc;
	desc.MipLevels = desc.ArraySize = 1;
	//TODO format
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.Width = width;
	desc.Height = height;

	ThrowIfFailed(renderer->dev->CreateTexture2D(&desc, nullptr, &textureResource));

	this->size.width() = width;
	this->size.height() = height;
}

void DirectX11Texture::initWithData(unsigned int width, unsigned int height, ColorFormat format, const void* data) {
	D3D11_TEXTURE2D_DESC desc;
	desc.MipLevels = desc.ArraySize = 1;
	//TODO format
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.Width = width;
	desc.Height = height;

	D3D11_SUBRESOURCE_DATA textureData = { 0 };
	textureData.pSysMem = data;
	textureData.SysMemPitch = desc.Width * 4;
	textureData.SysMemSlicePitch = 0;

	ThrowIfFailed(renderer->dev->CreateTexture2D(&desc, &textureData, &textureResource));

	this->size.width() = width;
	this->size.height() = height;
}

void DirectX11Texture::initFailed() {
	textureResource = nullptr;
}

} // namespace rhfw

