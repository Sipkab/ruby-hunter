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
 * OpenGl30FrameBuffer.cpp
 *
 *  Created on: 2015 m�rc. 18
 *      Author: sipka
 */

#include <opengl30/texture/OpenGl30Texture.h>
#include <gen/log.h>
#include <opengl30/buffer/OpenGl30FrameBuffer.h>
#include <opengl30/buffer/OpenGl30RenderBuffer.h>

namespace rhfw {

class TemporaryBinder {
private:
public:
	GLuint oldbuffer;
	GLuint buffer;
	OpenGl30Renderer* renderer;
	TemporaryBinder(OpenGl30Renderer* renderer, OpenGl30FrameBuffer* buffer)
			: renderer { renderer }, oldbuffer { buffer->getRenderer()->getBoundFrameBufferName() }, buffer { buffer->getGlName() } {
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

void OpenGl30FrameBuffer::setColorAttachment(RenderTargetType type, Resource<RenderObject> obj) {
	switch (type) {
		case RenderTargetType::TEXTURE: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& texture = static_cast<OpenGl30Texture&>(*obj);

			ASSERT(texture.getGlName() != 0) << "Gl name is 0";

			renderer->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getGlName(), 0);
			CHECK_GL_ERROR();

			break;
		}
		case RenderTargetType::RENDERBUFFER: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& buffer = static_cast<OpenGl30RenderBuffer&>(*obj);

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
void OpenGl30FrameBuffer::setDepthStencilAttachment(RenderTargetType type, Resource<RenderObject> obj) {
	switch (type) {
		case RenderTargetType::RENDERBUFFER: {
			ASSERT(obj != nullptr) << "No object specified with render target type: " << type;
			auto& buffer = static_cast<OpenGl30RenderBuffer&>(*obj);
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

OpenGl30FrameBuffer::OpenGl30FrameBuffer(OpenGl30Renderer* renderer)
		: renderer { renderer } {
}
OpenGl30FrameBuffer::~OpenGl30FrameBuffer() {
	ASSERT(name == 0) << "Framebuffer wasn't destroyed: " << name;
}
void OpenGl30FrameBuffer::bindForDrawing() {
	if (renderer->boundFrameBuffer != name) {
		//LOGI("Bind framebuffer: " <<  name);
		renderer->glBindFramebuffer(GL_FRAMEBUFFER, name);
		CHECK_GL_ERROR();
		renderer->boundFrameBuffer = name;
	}
}

void OpenGl30FrameBuffer::finishDrawing() {
	if (renderer->boundFrameBuffer == name) {
		//TODO bind back previous instead of zero
		//LOGI("Unbind framebuffer: " <<  name);

		//zero is not always the default framebuffer. We actually don't need to unbind it after all
//		renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		CHECK_GL_ERROR();
//		renderer->boundFrameBuffer = 0;
	}
}

render::ViewPort OpenGl30FrameBuffer::getDefaultViewPort() {
	//TODO really should redo this
	render::RenderObject* ro = getDescriptor().getColorTarget();
	render::SizedObject* obj =
			getDescriptor().getColorType() == RenderTargetType::TEXTURE ?
					static_cast<render::SizedObject*>(static_cast<render::Texture*>(ro)) :
					static_cast<render::SizedObject*>(static_cast<render::RenderBuffer*>(ro));
	return {0, 0, obj->getWidth(), obj->getHeight()};
}

void OpenGl30FrameBuffer::setAttachments() {
	TemporaryBinder binder { renderer, this };

	const render::RenderTargetDescriptor& desc = getDescriptor();

	setColorAttachment(desc.getColorType(), desc.getColorTarget());
	setDepthStencilAttachment(desc.getDepthStencilType(), desc.getDepthStencilTarget());

	ASSERT(renderer->glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) << "Framebuffer is incomplete";
}
bool OpenGl30FrameBuffer::load() {
	ASSERT(name == 0) << "Framebuffer already created";

	renderer->glGenFramebuffers(1, &name);
	CHECK_GL_ERROR();
	LOGD()<< "Created frame buffer with name: " << name;

	setAttachments();

	return true;
}

bool OpenGl30FrameBuffer::reload() {
	setAttachments();
	return true;
}

void OpenGl30FrameBuffer::free() {
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

