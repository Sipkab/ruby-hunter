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
 * OpenGlEs20FrameBuffer.h
 *
 *  Created on: 2015 m�rc. 18
 *      Author: sipka
 */

#ifndef GLFRAMEBUFFER_H_
#define GLFRAMEBUFFER_H_

#include <opengles20/OpenGlEs20Renderer.h>
#include <framework/render/RenderTarget.h>

#include <gen/configuration.h>

namespace rhfw {

class OpenGlEs20Texture;

class OpenGlEs20FrameBuffer: public render::RenderTarget {
private:
	GLuint name = 0;
	OpenGlEs20Renderer* renderer;

	void setColorAttachment(RenderTargetType type, Resource<RenderObject> obj);
	void setDepthStencilAttachment(RenderTargetType type, Resource<RenderObject> obj);

	void setAttachments();
protected:
	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;
public:

	OpenGlEs20FrameBuffer(OpenGlEs20Renderer* renderer);
	~OpenGlEs20FrameBuffer();

	virtual void bindForDrawing() override;
	virtual void finishDrawing() override;
	virtual render::ViewPort getDefaultViewPort() override;

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

#endif /* GLFRAMEBUFFER_H_ */
