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
 * GLTexture.cpp
 *
 *  Created on: 2014.06.12.
 *      Author: sipka
 */

#include <framework/render/Texture.h>

#include <opengles20/texture/OpenGlEs20Texture.h>

#include <gen/log.h>
#include <gen/types.h>
#include <opengles20/buffer/OpenGlEs20FrameBuffer.h>

namespace rhfw {

//TODO assign explicit gl values to enums
inline static GLuint translateGlInternalFormat(ColorFormat format) {
	switch (format) {
		case ColorFormat::RGBA_8888:
			return GL_RGBA;
		case ColorFormat::RGB_888:
			return GL_RGB;
		case ColorFormat::RGB_565:
			return GL_RGB;
		case ColorFormat::RGBA_4444:
			return GL_RGBA;
		case ColorFormat::A_8:
			return GL_ALPHA;
		default: {
			THROW()<< "Invalid color format: " << format;
			return -1;
		}
	}
}
inline static GLuint translateGlType(ColorFormat format) {
	switch (format) {
		case ColorFormat::RGBA_8888:
			return GL_UNSIGNED_BYTE;
		case ColorFormat::RGB_888:
			return GL_UNSIGNED_BYTE;
		case ColorFormat::RGB_565:
			return GL_UNSIGNED_SHORT_5_6_5;
		case ColorFormat::RGBA_4444:
			return GL_UNSIGNED_SHORT_4_4_4_4;
		case ColorFormat::A_8:
			return GL_UNSIGNED_BYTE;
		default: {
			THROW()<< "Invalid color format: " << format;
			return -1;
		}
	}
}

OpenGlEs20Texture::OpenGlEs20Texture(OpenGlEs20Renderer* renderer)
		: renderer { renderer } {
}
OpenGlEs20Texture::~OpenGlEs20Texture() {
}

bool OpenGlEs20Texture::applyTextureData() {
	auto* in = getInputSource();
	ASSERT(in != nullptr) << "Input source missing for texture";

	in->apply(this);
	return name != 0;
}
bool OpenGlEs20Texture::load() {
	renderer->glGenTextures(1, &name);
	CHECK_GL_ERROR();
	ASSERT(name != 0) << "Failed to create texture name";
	if (name == 0) {
		return false;
	}

	return applyTextureData();
}
void OpenGlEs20Texture::free() {
	unbind();

	renderer->glDeleteTextures(1, &name);
	CHECK_GL_ERROR();
	name = 0;
}
bool OpenGlEs20Texture::reload() {
	return applyTextureData();
}
GLint OpenGlEs20Texture::bind() {
	ASSERT(name != 0);
	auto& usedstart = renderer->textureBindUsedStart;

	if (bindPtr == nullptr) {
		auto& freestart = renderer->textureBindFreeStart;
		auto& usedend = renderer->textureBindUsedEnd;
		//need new binding
		if (freestart.next == &renderer->textureBindFreeEnd) {
			//need to steal binding
			bindPtr = usedend.prev;

			//remove binding reference from previous texture
			bindPtr->texture->bindPtr = nullptr;

			//remove from end of used list
			usedend.prev = bindPtr->prev;
			bindPtr->prev->next = &usedend;
		} else {
			//has free texture units
			bindPtr = freestart.next;

			//remove from free list start
			freestart.next = bindPtr->next;
			bindPtr->next->prev = &freestart;
		}
		//insert into used list start
		bindPtr->next = usedstart.next;
		bindPtr->prev = &usedstart;
		usedstart.next->prev = bindPtr;
		usedstart.next = bindPtr;

		bindPtr->texture = this;

		//LOGD("New bind texture %s to GL_TEXTURE%d", TOSTRING(getResourceId()), bindPtr->textureUnit - GL_TEXTURE0);

		renderer->glActiveTexture(bindPtr->textureUnit);
		CHECK_GL_ERROR();
		renderer->glBindTexture(glTextureTarget, name);
		CHECK_GL_ERROR();
	} else {
		//has binding already
		if (usedstart.next != bindPtr) {
			// LRU, move to start

			//remove from current position
			bindPtr->prev->next = bindPtr->next;
			bindPtr->next->prev = bindPtr->prev;

			//insert into used list start
			bindPtr->next = usedstart.next;
			bindPtr->prev = &usedstart;
			usedstart.next->prev = bindPtr;
			usedstart.next = bindPtr;
		}
	}
	return bindPtr->textureUnit - GL_TEXTURE0;
}
void OpenGlEs20Texture::unbind() {
	ASSERT(name != 0);
	if (bindPtr != nullptr) {
		auto& freestart = renderer->textureBindFreeStart;
		//LOGD("Unbind texture %s from GL_TEXTURE%d", TOSTRING(getResourceId()), bindPtr->textureUnit - GL_TEXTURE0);
		//move to free from used list

		//remove from current position
		bindPtr->prev->next = bindPtr->next;
		bindPtr->next->prev = bindPtr->prev;

		//insert into free list start
		bindPtr->next = freestart.next;
		bindPtr->prev = &freestart;
		freestart.next->prev = bindPtr;
		freestart.next = bindPtr;

		bindPtr = nullptr;
	}
}

void OpenGlEs20Texture::init2DDefault() {
	glTextureTarget = GL_TEXTURE_2D;

	if (bindPtr == nullptr) {
		bind();
	} else {
		GLint texunit = bind();
		//TODO keep track of active texture unit
		renderer->glActiveTexture(GL_TEXTURE0 + texunit);
		CHECK_GL_ERROR();
	}

	//ios npot textures: GL_LINEAR * 2 && GL_CLAMP_TO_EDGE * 2
	//http://stackoverflow.com/questions/11069441/non-power-of-two-textures-in-ios
	//http://stackoverflow.com/questions/4760174/rendering-to-non-power-of-two-texture-on-iphone
	renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_LINEAR_MIPMAP_LINEAR);
	CHECK_GL_ERROR();
	renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CHECK_GL_ERROR();

	//TODO ezeknek a parametereknek az allitasa
	renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERROR();
	renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERROR();
}

void OpenGlEs20Texture::initAsEmpty(unsigned int width, unsigned int height, ColorFormat format) {
	init2DDefault();

	const int internalformat = translateGlInternalFormat(format);
	const int type = translateGlType(format);

	renderer->glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, type, nullptr);
	CHECK_GL_ERROR();

	this->size.width() = width;
	this->size.height() = height;
}

void OpenGlEs20Texture::initWithData(unsigned int width, unsigned int height, ColorFormat format, const void* data) {
	init2DDefault();

	const int internalformat = translateGlInternalFormat(format);
	const int type = translateGlType(format);

	renderer->glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, type, data);
	CHECK_GL_ERROR();

	this->size.width() = width;
	this->size.height() = height;
}

void OpenGlEs20Texture::initFailed() {
	renderer->glDeleteTextures(1, &name);
	CHECK_GL_ERROR();
	name = 0;
}

} // namespace rhfw

