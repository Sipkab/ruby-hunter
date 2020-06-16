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
 * OpenGlEs20Renderer.h
 *
 *  Created on: 2014.06.07.
 *      Author: sipka
 */

#ifndef GLRENDERER_H_
#define GLRENDERER_H_

#include <framework/render/Renderer.h>

#include <gen/configuration.h>
#include <gen/types.h>

#include <opengles20/glglue/glglueclass.h>

#if RHFW_DEBUG
#define CHECK_GL_ERROR_REND(renderer) if(GLenum _gl_error = renderer->glGetError()) { THROW() << "GL has error state: " << _gl_error; }
#else /* DEBUG */
#define CHECK_GL_ERROR_REND(renderer)
#endif
#if RHFW_DEBUG
#define CHECK_GL_ERROR() CHECK_GL_ERROR_REND(renderer)
#else /* DEBUG */
#define CHECK_GL_ERROR()
#endif

namespace rhfw {
template<typename >
class OpenGlEs20ShaderProgramBase;
class OpenGlEs20Texture;
class OpenGlEs20Renderer final: public render::Renderer, public GlGlueFunctions {
	friend class OpenGlEs20IndexBuffer;
	friend class OpenGlEs20VertexBuffer;
	friend class OpenGlEs20FrameBuffer;
	friend class OpenGlEs20RenderBuffer;
	friend class OpenGlEs20Texture;
	template<typename >
	friend class OpenGlEs20ShaderProgramBase;
private:
	class TextureBindEntry {
	public:
		//pointer to the bound texture
		OpenGlEs20Texture* texture;
		//GL_TEXTURE* value
		GLenum textureUnit;

		TextureBindEntry* prev;
		TextureBindEntry* next;
	};

	class LinkEntry {
	public:
		LinkEntry* prev = nullptr;
		LinkEntry* next = nullptr;

	};
	class VertexAttribArrayState: public LinkEntry {
	public:
		GLint index;

		bool enabled() const {
			return prev != nullptr;
		}
	};

	static GLenum convertBufferUsageType(BufferType type) {
		switch (type) {
			case BufferType::DYNAMIC: {
				return GL_DYNAMIC_DRAW;
			}
			case BufferType::IMMUTABLE: {
				return GL_STATIC_DRAW;
			}
			default: {
				THROW()<<"unknown render buffer type: " << type;
				return GL_DYNAMIC_DRAW;
			}
		}
	}

	GLuint boundIndexBuffer = 0;
	GLuint boundVertexBuffer = 0;
	GLuint boundFrameBuffer = 0;
	GLuint boundRenderBuffer = 0;

	GLenum glTopology = GL_TRIANGLES;

	GLint propertyMaxCombinedTextureImageUnits = 0;
	GLint propertyMaxVertexAttribs = 0;

	TextureBindEntry* textureBindAllocatedEntries = nullptr;
	TextureBindEntry textureBindFreeStart;
	TextureBindEntry textureBindFreeEnd;
	TextureBindEntry textureBindUsedStart;
	TextureBindEntry textureBindUsedEnd;

	VertexAttribArrayState* vaaStates = nullptr;
	LinkEntry vaaEnabledStart;
	LinkEntry vaaEnabledEnd;

	render::ShaderProgram* activeProgram = nullptr;

	virtual void setDepthTestImpl(bool enabled) override;
	virtual void setFaceCullingImpl(bool enabled) override;
	virtual void setCullToFrontFaceImpl(bool isFront) override;
	virtual void setViewPortImpl(const render::ViewPort& vp) override;

	virtual render::Texture* createTextureImpl() override;
	virtual render::VertexBuffer* createVertexBufferImpl() override;
	virtual render::IndexBuffer* createIndexBufferImpl() override;
	virtual render::RenderTarget* createRenderTargetImpl() override;
	virtual render::RenderBuffer* createRenderBufferImpl() override;

	VertexAttribArrayState* enableVertexAttribArray(GLint index);
	void disableVertexAttribArrayStartingAt(VertexAttribArrayState* vaa);
protected:
	virtual void activateRenderTarget(RenderTargetStackEntry* target) override;

	virtual bool load() override;
	virtual void free() override;
public:

	OpenGlEs20Renderer();
	virtual ~OpenGlEs20Renderer();

	virtual void clearColor(const Color& color) override;
	virtual void clearDepthBuffer() override;
	virtual void clearColorDepthBuffer(const Color& color) override;

	virtual void setTopology(Topology topology) override {
		Renderer::setTopology(topology);
		switch (topology) {
			case Topology::TRIANGLES: {
				this->glTopology = GL_TRIANGLES;
				break;
			}
			case Topology::TRIANGLE_STRIP: {
				this->glTopology = GL_TRIANGLE_STRIP;
				break;
			}
			default: {
				break;
			}
		}
	}
	GLenum getGlTopology() const {
		return glTopology;
	}

	GLuint getBoundIndexBufferName() const {
		return boundIndexBuffer;
	}
	GLuint getBoundVertexBufferName() const {
		return boundVertexBuffer;
	}
	GLuint getBoundFrameBufferName() const {
		return boundFrameBuffer;
	}
	GLuint getBoundRenderBufferName() const {
		return boundRenderBuffer;
	}

	void setBoundFrameBufferName(GLuint fbname) {
		this->boundFrameBuffer = fbname;
	}

	render::ShaderProgram* getActiveProgram() const {
		return activeProgram;
	}

};
}
 // namespace rhfw

#endif /* GLRENDERER_H_ */
