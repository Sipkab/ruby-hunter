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
 * GLTexture.h
 *
 *  Created on: 2014.06.12.
 *      Author: sipka
 */

#ifndef GLTEXTURE_H_
#define GLTEXTURE_H_

#include <framework/render/Texture.h>

#include <opengles20/OpenGlEs20Renderer.h>

#include <gen/configuration.h>

namespace rhfw {

class OpenGlEs20Texture final: public render::Texture {
public:
	class BindEntry;
private:

	GLenum glTextureTarget = GL_TEXTURE_2D;

	bool applyTextureData();
protected:
	GLuint name = 0;

	OpenGlEs20Renderer::TextureBindEntry* bindPtr = nullptr;
	OpenGlEs20Renderer* renderer;

	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;

	void init2DDefault();

	virtual void initAsEmpty(unsigned int width, unsigned int height, ColorFormat format) override;
	virtual void initWithData(unsigned int width, unsigned int height, ColorFormat format, const void* data) override;

	virtual void initFailed() override;
public:
	OpenGlEs20Texture(OpenGlEs20Renderer* renderer);
	virtual ~OpenGlEs20Texture();

	GLint bind();
	void unbind();

	GLuint getGlName() const {
		return name;
	}
	OpenGlEs20Renderer* getRenderer() const {
		return renderer;
	}

	/**
	 * target: GL_TEXTURE_2D, GL_TEXTURE_EXTERNAL_OES
	 */
	void initGlTextureTarget(GLenum target) {
		glTextureTarget = target;
	}
};

}

#endif /* GLTEXTURE_H_ */
