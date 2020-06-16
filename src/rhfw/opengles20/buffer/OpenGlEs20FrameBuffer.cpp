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
 * OpenGlEs20FrameBuffer.cpp
 *
 *  Created on: 2015 m�rc. 18
 *      Author: sipka
 */

#include <opengles20/texture/OpenGlEs20Texture.h>
#include <gen/log.h>
#include <opengles20/buffer/OpenGlEs20FrameBuffer.h>
#include <opengles20/buffer/OpenGlEs20RenderBuffer.h>

namespace rhfw {

class TemporaryBinder {
private:
public:
	OpenGlEs20Renderer* renderer;
	GLuint oldbuffer;
	GLuint buffer;

	TemporaryBinder(OpenGlEs20FrameBuffer* buffer)
			: renderer(buffer->getRenderer()), oldbuffer { buffer->getRenderer()->getBoundFrameBufferName() }, buffer { buffer->getGlName() } {
		if (this->buffer != oldbuffer) {
			renderer->glBindFramebuffer(GL_FRAMEBUFFER, buffer->getGlName());
			CHECK_GL_ERROR();
		}
	}
	~TemporaryBinder() {
		if (this->buffer != oldbuffer) {
			renderer->glBindFramebuffer(GL_FRAMEBUFFER, oldbuffer);
			CHECK_GL_ERROR();
		}
	}
};

void OpenGlEs20FrameBuffer::setColorAttachment(RenderTargetType type, Resource<RenderObject> obj) {
	switch (type) {
		case RenderTargetType::TEXTURE: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& texture = static_cast<OpenGlEs20Texture&>(*obj);

			ASSERT(texture.getGlName() != 0) << "Gl name is 0";

			renderer->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getGlName(), 0);
			CHECK_GL_ERROR();

			break;
		}
		case RenderTargetType::RENDERBUFFER: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& buffer = static_cast<OpenGlEs20RenderBuffer&>(*obj);

			ASSERT(buffer.getGlName() != 0) << "Gl name is 0";

			renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer.getGlName());
			CHECK_GL_ERROR();

			break;
		}
		case RenderTargetType::UNUSED: {
			//unbind Attachment
			renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, 0);
			CHECK_GL_ERROR();
			break;
		}
		default: {
			THROW()<< "Unsupported render target type: " << type;
			break;
		}
	}
}
void OpenGlEs20FrameBuffer::setDepthStencilAttachment(RenderTargetType type, Resource<RenderObject> obj) {
	switch (type) {
		case RenderTargetType::RENDERBUFFER: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& buffer = static_cast<OpenGlEs20RenderBuffer&>(*obj);
			ASSERT(buffer.getGlName() != 0) << "Gl name is 0";

			switch (buffer.getType()) {
				case RenderBufferType::COLOR: {
					renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer.getGlName());
					CHECK_GL_ERROR();
					break;
				}
				case RenderBufferType::DEPTH: {
					renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer.getGlName());
					CHECK_GL_ERROR();
					break;
				}
				case RenderBufferType::STENCIL: {
					renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.getGlName());
					CHECK_GL_ERROR();
					break;
				}
					//TODO support DEPTH_STENCIL
				default: {
					THROW()<< "Unsupported attachment: " << buffer.getType();
					break;
				}
			}
			break;
		}
		case RenderTargetType::TEXTURE: {
			THROW()<< "Attaching texture for depth-stencil is unsupported";
			break;
		}
		case RenderTargetType::UNUSED: {
			renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
			CHECK_GL_ERROR();
			renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
			CHECK_GL_ERROR();
			break;
		}
		default: {
			THROW() << "Unsupported render target type: " << type;
			break;
		}
	}
}

OpenGlEs20FrameBuffer::OpenGlEs20FrameBuffer(OpenGlEs20Renderer* renderer)
		: renderer { renderer } {
}
OpenGlEs20FrameBuffer::~OpenGlEs20FrameBuffer() {
	ASSERT(name == 0) << "Framebuffer wasn't destroyed: " << name;
}
void OpenGlEs20FrameBuffer::bindForDrawing() {
	if (renderer->boundFrameBuffer != name) {
		//LOGI("Bind framebuffer: " <<  name);
		renderer->glBindFramebuffer(GL_FRAMEBUFFER, name);
		CHECK_GL_ERROR();
		renderer->boundFrameBuffer = name;
	}
}

void OpenGlEs20FrameBuffer::finishDrawing() {
	if (renderer->boundFrameBuffer == name) {
		//TODO bind back previous instead of zero
		//LOGI("Unbind framebuffer: " <<  name);

		//zero is not always the default framebuffer. We actually don't need to unbind it after all
//		renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		CHECK_GL_ERROR();
//		renderer->boundFrameBuffer = 0;
	}
}

render::ViewPort OpenGlEs20FrameBuffer::getDefaultViewPort() {
	//TODO really should redo this
	render::RenderObject* ro = getDescriptor().getColorTarget();
	render::SizedObject* obj =
			getDescriptor().getColorType() == RenderTargetType::TEXTURE ?
					static_cast<render::SizedObject*>(static_cast<render::Texture*>(ro)) :
					static_cast<render::SizedObject*>(static_cast<render::RenderBuffer*>(ro));
	return {0, 0, obj->getWidth(), obj->getHeight()};
}

void OpenGlEs20FrameBuffer::setAttachments() {
	TemporaryBinder binder { this };

	const render::RenderTargetDescriptor& desc = getDescriptor();

	setColorAttachment(desc.getColorType(), desc.getColorTarget());
	setDepthStencilAttachment(desc.getDepthStencilType(), desc.getDepthStencilTarget());

	ASSERT(renderer->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "Framebuffer is incomplete";
}
bool OpenGlEs20FrameBuffer::load() {
	ASSERT(name == 0) << "Framebuffer already created";

	renderer->glGenFramebuffers(1, &name);
	CHECK_GL_ERROR();
	LOGD()<< "Created frame buffer with name: " << name;

	setAttachments();

	return true;
}

bool OpenGlEs20FrameBuffer::reload() {
	setAttachments();
	return true;
}

void OpenGlEs20FrameBuffer::free() {
	ASSERT(name != 0) << "Framebuffer is not created";

	renderer->glDeleteFramebuffers(1, &name);
	CHECK_GL_ERROR();
	LOGD()<< "Destroyed frame buffer with name: " << name;

	if (renderer->boundFrameBuffer == name) {
		renderer->boundFrameBuffer = 0;
	}
	name = 0;
}

} // namespace rhfw

