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
 * OpenGlEs20RenderBuffer.h
 *
 *  Created on: 2015 apr. 21
 *      Author: sipka
 */

#ifndef GLRENDERBUFFER_H_
#define GLRENDERBUFFER_H_

#include <framework/render/RenderBuffer.h>
#include <opengles20/OpenGlEs20Renderer.h>

#include <gen/configuration.h>

namespace rhfw {

class OpenGlEs20Texture;

class OpenGlEs20RenderBuffer: public render::RenderBuffer {
private:
	GLuint name = 0;
	OpenGlEs20Renderer* renderer;

	void applyConfig();
protected:
	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;
public:
	OpenGlEs20RenderBuffer(OpenGlEs20Renderer* renderer);
	~OpenGlEs20RenderBuffer();

	void bind();

	bool isCreated() const {
		return name != 0;
	}
	GLuint getGlName() const {
		return name;
	}
	OpenGlEs20Renderer* getRenderer() const {
		return renderer;
	}
};
}

#endif /* GLRENDERBUFFER_H_ */
