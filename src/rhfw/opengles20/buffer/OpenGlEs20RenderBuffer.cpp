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
 * OpenGlEs20RenderBuffer.cpp
 *
 *  Created on: 2015 �pr. 21
 *      Author: sipka
 */

#include <opengles20/texture/OpenGlEs20Texture.h>

#include <gen/log.h>
#include <opengles20/buffer/OpenGlEs20RenderBuffer.h>
namespace rhfw {

static GLenum convertType(RenderBufferType type) {
	switch (type) {
		case RenderBufferType::COLOR:
			return GL_RGB565;
		case RenderBufferType::DEPTH:
			return GL_DEPTH_COMPONENT16;
		case RenderBufferType::STENCIL:
			return GL_STENCIL_INDEX8;
		default: {
			THROW()<< "Unknown render buffer type: " << type;
			return GL_RGB565;
		}
	}
}

OpenGlEs20RenderBuffer::OpenGlEs20RenderBuffer(OpenGlEs20Renderer* renderer)
		: renderer { renderer } {
}
OpenGlEs20RenderBuffer::~OpenGlEs20RenderBuffer() {
	ASSERT(name == 0) << "OpenGlEs20RenderBuffer wasn't destroyed: " << name;
}

void OpenGlEs20RenderBuffer::bind() {
	ASSERT(isCreated()) << "OpenGlEs20RenderBuffer is not created, can't bind";

	if (renderer->boundRenderBuffer != name) {
		renderer->glBindRenderbuffer(GL_RENDERBUFFER, name);
		CHECK_GL_ERROR();
		renderer->boundRenderBuffer = name;
	}
}
bool OpenGlEs20RenderBuffer::load() {
	ASSERT(name == 0) << "render buffer already created: " << name;
	renderer->glGenRenderbuffers(1, &name);
	CHECK_GL_ERROR();

	applyConfig();

	LOGD()<< "Created render buffer with name: " << name;
	return true;
}

void OpenGlEs20RenderBuffer::free() {
	ASSERT(name != 0) << "Render buffer wasnt created, cant destroy";
	if (renderer->boundRenderBuffer == name) {
		renderer->boundRenderBuffer = 0;
	}

	renderer->glDeleteRenderbuffers(1, &name);
	CHECK_GL_ERROR();
	LOGD()<< "Destroyed render buffer with name: " << name;

	name = 0;
}

bool OpenGlEs20RenderBuffer::reload() {
	applyConfig();
	return true;
}

void OpenGlEs20RenderBuffer::applyConfig() {
	bind();

	renderer->glRenderbufferStorage(GL_RENDERBUFFER, convertType(getType()), getWidth(), getHeight());
	CHECK_GL_ERROR();
}

} // namespace rhfw

