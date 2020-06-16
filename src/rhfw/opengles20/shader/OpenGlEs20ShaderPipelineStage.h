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
 * OpenGlEs20ShaderPipelineStage.h
 *
 *  Created on: 2016. marc. 31.
 *      Author: sipka
 */

#ifndef OPENGLES20_SHADER_OPENGLES20SHADERPIPELINESTAGE_H_
#define OPENGLES20_SHADER_OPENGLES20SHADERPIPELINESTAGE_H_

#include <framework/render/ShaderPipelineStage.h>

namespace rhfw {

template<typename ParentType = render::ShaderPipelineStage>
class OpenGlEs20ShaderPipelineStage: public ParentType {
protected:
	GLuint shader = 0;
	OpenGlEs20Renderer* renderer;
public:
	OpenGlEs20ShaderPipelineStage(OpenGlEs20Renderer* renderer)
			: renderer { renderer } {
	}

	GLuint getGlName() const {
		return shader;
	}
	OpenGlEs20Renderer* getRenderer() const {
		return renderer;
	}
};

}  // namespace rhfw

#endif /* OPENGLES20_SHADER_OPENGLES20SHADERPIPELINESTAGE_H_ */
