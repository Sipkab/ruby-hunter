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
#ifndef DIRECTXSHADERPIPELINESTAGE_H_
#define DIRECTXSHADERPIPELINESTAGE_H_

#include <framework/render/ShaderPipelineStage.h>
#include <directx11/DirectX11Renderer.h>

namespace rhfw {

template<typename D11ShaderPtr, typename ParentType = render::ShaderPipelineStage>
class DirectX11ShaderPipelineStage: public ParentType {
protected:
	DirectX11Renderer* renderer;
	D11ShaderPtr* shader;

	const char* shaderdata;
	int shadersize;
public:
	DirectX11ShaderPipelineStage(DirectX11Renderer* renderer)
			: renderer { renderer } {
	}

	D11ShaderPtr* getDirectX11Name() {
		return shader;
	}
	DirectX11Renderer* getRenderer() const {
		return renderer;
	}

	const char* getShaderData() const {
		return shaderdata;
	}
	int getShaderDataSize() const {
		return shadersize;
	}
};

} // namespace rhfw

#endif /*DIRECTXSHADERPIPELINESTAGE_H_*/

