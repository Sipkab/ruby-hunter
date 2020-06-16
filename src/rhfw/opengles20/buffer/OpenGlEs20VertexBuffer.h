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
 * OpenGlEs20VertexBuffer.h
 *
 *  Created on: 2015 febr. 6
 *      Author: sipka
 */

#ifndef GLVERTEXBUFFER_H_
#define GLVERTEXBUFFER_H_

#include <opengles20/OpenGlEs20Renderer.h>
#include <framework/render/VertexBuffer.h>
#include <framework/render/VertexBuffer.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {

class OpenGlEs20VertexBuffer final : public render::VertexBuffer {
	friend class OpenGlEs20Renderer;
private:

	GLuint name = 0;
	OpenGlEs20Renderer* renderer;

	virtual void allocateDataImpl(const void* data, unsigned int bytecount, BufferType bufferType) override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		//LOGI("allocateDataImpl %u %d", bytecount, name);
		activate();

		renderer->glBufferData(GL_ARRAY_BUFFER, bytecount, data, OpenGlEs20Renderer::convertBufferUsageType(bufferType));
		CHECK_GL_ERROR();
	}
	virtual void updateRegionImpl(const void* data, unsigned int start, unsigned int bytecount) override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		//LOGI("updateRegionImpl %u - %u %d", start, bytecount, name);
		activate();

		renderer->glBufferSubData(GL_ARRAY_BUFFER, start, bytecount, data);
		CHECK_GL_ERROR();
	}
protected:
	virtual bool loadImpl() override {
		ASSERT(name == 0) << "Buffer is already initialized: " << name;

		renderer->glGenBuffers(1, &name);
		CHECK_GL_ERROR();
		ASSERT(name != 0) << "Failed to create buffer";

		return true;
	}
	virtual void freeImpl() override {

		ASSERT(name != 0) << "Buffer is not initialized: " << name;

		if (renderer->boundVertexBuffer == name) {
			renderer->boundVertexBuffer = 0;
		}
		renderer->glDeleteBuffers(1, &name);
		CHECK_GL_ERROR();
		name = 0;
	}
	virtual bool reloadImpl() override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		return true;
	}

	OpenGlEs20VertexBuffer(OpenGlEs20Renderer* renderer)
			: renderer { renderer } {
	}
public:
	~OpenGlEs20VertexBuffer() {
		ASSERT(name == 0) << "Buffer wasn't destroyed: " << name;
	}

	void activate() {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;

		if (renderer->boundVertexBuffer != name) {
			renderer->glBindBuffer(GL_ARRAY_BUFFER, name);
			CHECK_GL_ERROR();
			renderer->boundVertexBuffer = name;
		}
	}
	void deactivate() {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		if (renderer->boundVertexBuffer == name) {
			renderer->glBindBuffer(GL_ARRAY_BUFFER, 0);
			CHECK_GL_ERROR();
			renderer->boundVertexBuffer = 0;
		}
	}
	GLuint getGlName() const {
		return name;
	}
	OpenGlEs20Renderer* getRenderer() const {
		return renderer;
	}
};

}

#endif /* GLVERTEXBUFFER_H_ */
