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
#ifndef DIRECTXSHADERPROGRAMBASE_H_
#define DIRECTXSHADERPROGRAMBASE_H_

#include <directx11/DirectX11Renderer.h>
#include <directx11/DirectX11ShaderPipelineStage.h>
#include <framework/render/ShaderProgram.h>
#include <framework/geometry/Matrix.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

template<typename ShaderParentType = render::ShaderProgram>
class DirectX11ShaderProgramBase: public ShaderParentType {
private:
protected:
	DirectX11Renderer* renderer;

	template<typename VertexShaderType, typename FragmentShaderType>
	bool executeLoad() {
		ShaderParentType::vertexShader = renderer->getShaderPipelineStage(VertexShaderType::PIPELINE_STAGE);
		ShaderParentType::fragmentShader = renderer->getShaderPipelineStage(FragmentShaderType::PIPELINE_STAGE);
		return true;
	}
	virtual bool load() override {
		return true;
	}
	virtual void free() override {
		if (renderer->activePixelShader == ShaderParentType::fragmentShader) {
			renderer->devcon->PSSetShader(nullptr, nullptr, 0);
			renderer->activePixelShader = nullptr;
		}
		if (renderer->activeVertexShader == ShaderParentType::vertexShader) {
			renderer->devcon->VSSetShader(nullptr, nullptr, 0);
			renderer->activeVertexShader = nullptr;
		}

		ShaderParentType::vertexShader = nullptr;
		ShaderParentType::fragmentShader = nullptr;
	}
public:
	DirectX11ShaderProgramBase(DirectX11Renderer* renderer)
			: ShaderParentType { }, renderer { renderer } {
	}

	virtual void draw(int first, int count) override {
		//ASSERT(isInUse(), "Program is not in use");
		renderer->devcon->Draw(count, first);
	}

	virtual void drawIndexed(int first, int count) override {
		//ASSERT(isInUse(), "Program is not in use");
		renderer->devcon->DrawIndexed(count, first, 0);
	}

	DirectX11Renderer* getRenderer() {
		return renderer;
	}
};

}

#endif /*DIRECTXSHADERPROGRAMBASE_H_*/
