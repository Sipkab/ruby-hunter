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
 */
#ifndef DIRECTXTEXTURE_H_
#define DIRECTXTEXTURE_H_

#include <framework/render/Texture.h>

#include <d3d11.h>

#include <gen/configuration.h>
namespace rhfw {
class DirectX11Renderer;
class DirectX11Texture: public render::Texture {
private:
	DirectX11Renderer* renderer;

	bool createTextureResources();
protected:
	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;

	virtual void initAsEmpty(unsigned int width, unsigned int height, ColorFormat format) override;
	virtual void initWithData(unsigned int width, unsigned int height, ColorFormat format, const void* data) override;

	virtual void initFailed() override;
public:
	DirectX11Texture(DirectX11Renderer* renderer)
			: renderer { renderer } {
		ASSERT(renderer != nullptr) << "renderer is null";
	}

	ID3D11SamplerState* samplerState = nullptr;
	ID3D11Texture2D* textureResource = nullptr;
	ID3D11ShaderResourceView* shaderResource = nullptr;

	virtual ~DirectX11Texture() {
	}

	DirectX11Renderer* getRenderer() {
		return renderer;
	}
	ID3D11Texture2D* getTextureName() {
		return textureResource;
	}
};
}

#endif /* DIRECTXTEXTURE_H_ */
