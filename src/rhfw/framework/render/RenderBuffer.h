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
 * RenderBuffer.h
 *
 *  Created on: 2016. marc. 27.
 *      Author: sipka
 */

#ifndef RENDER_RENDERBUFFER_H_
#define RENDER_RENDERBUFFER_H_

#include <framework/render/RenderObject.h>
#include <framework/render/SizedObject.h>

#include <gen/types.h>

namespace rhfw {
namespace render {

class RenderBuffer: public RenderObject, public SizedObject {
private:
	unsigned int width = 0;
	unsigned int height = 0;

	RenderBufferType type = RenderBufferType::UNKNOWN;
public:

	RenderBuffer& operator=(RenderBuffer&& o) {
		this->width = o.width;
		this->height = o.height;

		this->type = o.type;

		return *this;
	}

	void setWidth(unsigned int width) {
		this->width = width;
	}
	unsigned int getWidth() const override {
		return width;
	}
	void setHeight(unsigned int height) {
		this->height = height;
	}
	unsigned int getHeight() const override {
		return height;
	}

	void setType(RenderBufferType type) {
		this->type = type;
	}
	RenderBufferType getType() const {
		return this->type;
	}

	void set(unsigned int width, unsigned int height, RenderBufferType type) {
		this->width = width;
		this->height = height;
		this->type = type;
	}
};

}  // namespace render
}  // namespace rhfw
#endif /* RENDER_RENDERBUFFER_H_ */
