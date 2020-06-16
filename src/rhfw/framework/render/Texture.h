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
 * Texture.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RENDER_TEXTURE_H_
#define RENDER_TEXTURE_H_

#include <framework/render/RenderObject.h>
#include <framework/render/SizedObject.h>
#include <framework/geometry/Vector.h>

#include <framework/utils/InputSource.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/types.h>
#include <gen/renderers.h>

namespace rhfw {
class DirectX11Texture;
class OpenGlEs20Texture;
class OpenGl30Texture;
namespace render {

class Texture;

class TextureInitializer {
public:
	virtual ~TextureInitializer() = default;

	virtual void initAsEmpty(unsigned int width, unsigned int height, ColorFormat format) = 0;
	virtual void initWithData(unsigned int width, unsigned int height, ColorFormat format, const void* data) = 0;

	virtual void initFailed() = 0;
};

class TextureInputSource {
private:
protected:
	static TextureInitializer* asInitializer(Texture* texture);
public:
	virtual ~TextureInputSource() = default;
	virtual void apply(Texture* texture) = 0;

#if RENDERAPI_opengl30_AVAILABLE
	virtual void apply(OpenGl30Texture* texture);
#endif /* RENDERAPI_opengl30_AVAILABLE */

#if RENDERAPI_opengles20_AVAILABLE
	virtual void apply(OpenGlEs20Texture* texture);
#endif /* RENDERAPI_opengles20_AVAILABLE */

#if RENDERAPI_directx11_AVAILABLE
	virtual void apply(DirectX11Texture* texture);
#endif /* RENDERAPI_directx11_AVAILABLE */
};

class EmptyInputSource: public TextureInputSource {
private:
	Size2UI size;
	ColorFormat format;

protected:
	virtual void apply(Texture* texture) override {
		asInitializer(texture)->initAsEmpty(size.width(), size.height(), format);
	}
public:
	EmptyInputSource(const Size2UI& size, ColorFormat format)
			: size { size }, format { format } {
	}
	EmptyInputSource(unsigned int width, unsigned int height, ColorFormat format)
			: EmptyInputSource(Size2UI { width, height }, format) {
	}

	unsigned int getWidth() const {
		return size.width();
	}
	unsigned int getHeight() const {
		return size.height();
	}

	ColorFormat getColorFormat() const {
		return format;
	}

};

class Texture: public RenderObject, public SizedObject, protected TextureInitializer {
private:
	friend class TextureInputSource;

	TextureInputSource* input = nullptr;

protected:
	Size2UI size { 0, 0 };
public:
	Texture() = default;
	Texture(Texture&& o)
			: input { o.input } {
		o.input = nullptr;
	}
	Texture& operator=(Texture&& o) {
		ASSERT(this != &o) << "self move assignment";

		setInputSource(o.input);

		o.input = nullptr;
		return *this;
	}
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	~Texture() {
		delete input;
	}

	void setInputSource(TextureInputSource* source) {
		if (this->input != nullptr) {
			delete this->input;
		}
		this->input = source;
	}
	TextureInputSource* getInputSource() {
		return input;
	}

	unsigned int getWidth() const override {
		return size.width();
	}
	unsigned int getHeight() const override {
		return size.height();
	}
	Size2UI getSize() const {
		return size;
	}
};

inline TextureInitializer* TextureInputSource::asInitializer(Texture* texture) {
	return texture;
}

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_TEXTURE_H_ */
