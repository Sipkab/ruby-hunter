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
 * OpenGlEs20IndexBuffer.h
 *
 *  Created on: 2015 febr. 6
 *      Author: sipka
 */

#ifndef GLINDEXBUFFER_H_
#define GLINDEXBUFFER_H_

#include <opengles20/OpenGlEs20Renderer.h>
#include <framework/render/IndexBuffer.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class OpenGlEs20IndexBuffer final : public render::IndexBuffer {
	friend class OpenGlEs20Renderer;
private:
	GLuint name = 0;
	OpenGlEs20Renderer* renderer;

	virtual void allocateDataImpl(const void* data, unsigned int bytecount, BufferType bufferType) override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;

		activate();

		renderer->glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytecount, data, OpenGlEs20Renderer::convertBufferUsageType(bufferType));
		CHECK_GL_ERROR();
	}
	virtual void updateRegionImpl(const void* data, unsigned int start, unsigned int bytecount) override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;

		activate();
		renderer->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start, bytecount, data);
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

		if (renderer->boundIndexBuffer == name) {
			renderer->boundIndexBuffer = 0;
		}
		renderer->glDeleteBuffers(1, &name);
		CHECK_GL_ERROR();
		name = 0;
	}
	virtual bool reloadImpl() override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		return true;
	}

	OpenGlEs20IndexBuffer(OpenGlEs20Renderer* renderer)
			: renderer { renderer } {
	}
public:
	~OpenGlEs20IndexBuffer() {
		ASSERT(name == 0) << "Buffer wasn't destroyed: " << name;
	}

	virtual void activate() override {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;

		if (renderer->boundIndexBuffer != name) {
			renderer->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, name);
			CHECK_GL_ERROR();
			renderer->boundIndexBuffer = name;
		}
	}
	void deactivate() {
		ASSERT(name != 0) << "Buffer is not initialized: " << name;
		if (renderer->boundIndexBuffer == name) {
			renderer->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			CHECK_GL_ERROR();
			renderer->boundIndexBuffer = 0;
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

#endif /* GLINDEXBUFFER_H_ */
