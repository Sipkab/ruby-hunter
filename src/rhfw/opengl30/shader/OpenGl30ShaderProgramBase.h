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
 * OpenGl30ShaderProgramBase.h
 *
 *  Created on: 2016.02.10.
 *      Author: sipka
 */

#ifndef GLSHADERPROGRAMBASE_H_
#define GLSHADERPROGRAMBASE_H_

#include <framework/render/ShaderProgram.h>
#include <opengl30/OpenGl30Renderer.h>
#include <opengl30/shader/OpenGl30ShaderPipelineStage.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

template<typename ShaderParentType = render::ShaderProgram>
class OpenGl30ShaderProgramBase: public ShaderParentType {
protected:

	GLuint program = 0;
	OpenGl30Renderer* renderer;

	OpenGl30Renderer::VertexAttribArrayState* enableVertexAttribArray(GLint index) {
		return renderer->enableVertexAttribArray(index);
	}
	void disableStartingAt(OpenGl30Renderer::VertexAttribArrayState* vaa) {
		renderer->disableVertexAttribArrayStartingAt(vaa);
	}
	template<typename VertexShaderType, typename FragmentShaderType>
	bool executeLoad() {
		program = renderer->glCreateProgram();
		CHECK_GL_ERROR();
		ASSERT(program != 0) << "Couldn't create program";

		ShaderParentType::vertexShader = renderer->getShaderPipelineStage(VertexShaderType::PIPELINE_STAGE);
		ShaderParentType::fragmentShader = renderer->getShaderPipelineStage(FragmentShaderType::PIPELINE_STAGE);

		renderer->glAttachShader(program, static_cast<VertexShaderType*>(ShaderParentType::vertexShader)->getGlName());
		CHECK_GL_ERROR();
		renderer->glAttachShader(program, static_cast<FragmentShaderType*>(ShaderParentType::fragmentShader)->getGlName());
		CHECK_GL_ERROR();

		renderer->glLinkProgram(program);
		CHECK_GL_ERROR();

		renderer->glDetachShader(program, static_cast<VertexShaderType*>(ShaderParentType::vertexShader)->getGlName());
		CHECK_GL_ERROR();
		renderer->glDetachShader(program, static_cast<FragmentShaderType*>(ShaderParentType::fragmentShader)->getGlName());
		CHECK_GL_ERROR();

#if RHFW_DEBUG
		GLint res;
		renderer->glGetProgramiv(program, GL_LINK_STATUS, &res);
		CHECK_GL_ERROR();
		if (res == GL_FALSE) {
			GLchar errordesc[512];
			GLsizei size;
			renderer->glGetProgramInfoLog(program, sizeof errordesc, &size, errordesc);
			LOGWTF()<< "Failed to compile shader:\nError: " << errordesc;
			CHECK_GL_ERROR();
			renderer->glDeleteProgram(program);
			CHECK_GL_ERROR();
			THROW()<< "Couldn't link program";
			return false;
		}
#endif /* DEBUG */
		return true;
	}

	void free() override {
		if (renderer->activeProgram == this) {
			renderer->activeProgram = nullptr;
		}

		ShaderParentType::vertexShader = nullptr;
		ShaderParentType::fragmentShader = nullptr;

		renderer->glDeleteProgram(program);
		CHECK_GL_ERROR();
	}
	virtual void onUseProgram() {
	}
public:
	OpenGl30ShaderProgramBase(OpenGl30Renderer* renderer)
			: ShaderParentType { }, renderer { renderer } {
	}

	void useProgram() override {
		if (renderer->activeProgram != this) {
			/*if (renderer->activeProgram != nullptr) {
			 renderer->activeProgram->onUnUseProgram();
			 }*/
			renderer->glUseProgram(program);
			CHECK_GL_ERROR();
			renderer->activeProgram = this;
			onUseProgram();
		}
	}
	virtual void draw(int first, int count) override {
		ASSERT(isInUse()) << "Program is not in use";
		renderer->glDrawArrays(renderer->getGlTopology(), first, count);
		CHECK_GL_ERROR();
	}

	virtual void drawIndexed(int first, int count) override {
		ASSERT(isInUse()) << "Program is not in use";
		renderer->glDrawElements(renderer->getGlTopology(), count, GL_UNSIGNED_SHORT, (void*) (size_t) (0 + first * 2));
		CHECK_GL_ERROR();
	}

	bool isInUse() {
		return renderer->activeProgram == this;
	}
	GLuint getGlName() const {
		return program;
	}
	OpenGl30Renderer* getRenderer() const {
		return renderer;
	}
};

}

#endif /*GLSHADERPROGRAMBASE_H_*/
