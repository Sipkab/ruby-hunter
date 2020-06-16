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
 * ShaderProgram.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RENDER_SHADERPROGRAM_H_
#define RENDER_SHADERPROGRAM_H_

#include <framework/resource/TrackingResourceBlock.h>
#include <framework/render/InputLayout.h>
#include <framework/render/RenderObject.h>
#include <framework/utils/LinkedList.h>

#include <gen/types.h>

namespace rhfw {
namespace render {

class ShaderPipelineStage;
class InputLayout;

class ShaderProgram: public RenderObject {
	friend class Renderer;
protected:
	LinkedList<InputLayout> inputLayouts;
	virtual InputLayout* createInputLayoutImpl() = 0;

	AutoResource<render::ShaderPipelineStage> vertexShader;
	AutoResource<render::ShaderPipelineStage> fragmentShader;

	virtual void freeInContext() {
		for (auto* o : inputLayouts.nodes()) {
			auto* res = static_cast<TrackingResourceBlock<render::InputLayout>*>(o);
			if (res->isLoaded()) {
				res->get()->free();
			}
		}
	}
	virtual bool reloadInNewContext() {
		for (auto* o : inputLayouts.nodes()) {
			auto* res = static_cast<TrackingResourceBlock<render::InputLayout>*>(o);
			if (res->isLoaded()) {
				res->get()->load();
			}
		}
		return true;
	}

	virtual void moveResourcesFrom(ShaderProgram&& prog) {
		ASSERT(inputLayouts.isEmpty());
		inputLayouts = util::move(prog.inputLayouts);
		for (auto* o : inputLayouts.nodes()) {
			auto* res = static_cast<TrackingResourceBlock<render::InputLayout>*>(o);
			if (res->isLoaded()) {
				res->get()->free();
			}
		}
	}
	virtual void updateMovedResources() = 0;
public:

	Resource<InputLayout> createInputLayout() {
		return Resource<InputLayout> { new TrackingResourceBlock<render::InputLayout> { createInputLayoutImpl(), inputLayouts } };
	}

	virtual void useProgram() = 0;

	virtual void drawIndexed(int first, int count) = 0;
	virtual void draw(int first, int count) = 0;

	void drawAdvance(int& first, int count) {
		draw(first, count);
		first += count;
	}
	void drawCount(int count) {
		draw(0, count);
	}

	void drawIndexedAdvance(int& first, int count) {
		drawIndexed(first, count);
		first += count;
	}
	void drawIndexedCount(int count) {
		drawIndexed(0, count);
	}
};

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_SHADERPROGRAM_H_ */
