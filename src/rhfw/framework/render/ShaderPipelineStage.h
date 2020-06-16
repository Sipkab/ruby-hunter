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
 * ShaderPipelineStage.h
 *
 *  Created on: 2016. marc. 30.
 *      Author: sipka
 */

#ifndef FRAMEWORK_RENDER_SHADERPIPELINESTAGE_H_
#define FRAMEWORK_RENDER_SHADERPIPELINESTAGE_H_

#include <framework/render/RenderObject.h>

namespace rhfw {
namespace render {

class ShaderPipelineStage: public RenderObject {
	friend class Renderer;
private:
protected:
	virtual void moveResourcesFrom(ShaderPipelineStage&& prog) {
	}
	virtual void updateMovedResources() {
	}
	virtual void freeInContext() {
	}
	virtual bool reloadInNewContext() {
		return true;
	}
public:
};

}  // namespace render
}  // namespace rhfw

#endif /* FRAMEWORK_RENDER_SHADERPIPELINESTAGE_H_ */
