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
 * OpenGlEs20Renderer.cpp
 *
 *  Created on: 2014.06.07.
 *      Author: sipka
 */

#include <opengles20/OpenGlEs20Renderer.h>
#include <opengles20/texture/OpenGlEs20Texture.h>
#include <opengles20/buffer/OpenGlEs20IndexBuffer.h>
#include <opengles20/shader/OpenGlEs20ShaderProgramBase.h>

#include <gen/log.h>
#include <opengles20/buffer/OpenGlEs20FrameBuffer.h>
#include <opengles20/buffer/OpenGlEs20RenderBuffer.h>
#include <opengles20/buffer/OpenGlEs20VertexBuffer.h>

#include <gen/renderers.h>

namespace rhfw {

OpenGlEs20Renderer::OpenGlEs20Renderer()
		: Renderer { RenderConfig::OpenGlEs20, TextureLoadOrder::LAST_LINE_AT_ORIGIN, TextureUvOrientation::Y_DIR_UP,
				DepthRange::RANGE_NEG_1_TO_POS_1 } {
}
OpenGlEs20Renderer::~OpenGlEs20Renderer() {
}

bool OpenGlEs20Renderer::load() {
	if (!Renderer::load()) {
		return false;
	}

	GlGlueFunctions::load(getRenderingContext());

	CHECK_GL_ERROR_REND(this);

	LOGI()<< "OpenGL ES vendor: " << glGetString(GL_VENDOR);
	LOGI()<< "OpenGL ES renderer: " << glGetString(GL_RENDERER);
	LOGI()<< "OpenGL ES version: " << glGetString(GL_VERSION);
	LOGI()<< "OpenGL ES shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
	LOGI()<< "OpenGL ES extensions: " << glGetString(GL_EXTENSIONS);

	//GL_CCW is initial
	//glFrontFace(GL_CCW);
	//CHECK_GL_ERROR_REND(this);

	//GL_LESS is initial
	//glDepthFunc(GL_LEQUAL);
	//CHECK_GL_ERROR_REND(this);

	//GL_BLEND is disabled by default
	glEnable(GL_BLEND);
	CHECK_GL_ERROR_REND(this);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CHECK_GL_ERROR_REND(this);

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &propertyMaxCombinedTextureImageUnits);
	CHECK_GL_ERROR_REND(this);
	ASSERT(propertyMaxCombinedTextureImageUnits >= 8)
			<< "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS is less than defined in OpenGL ES 2.0 standard (8): "
			<< propertyMaxCombinedTextureImageUnits;
	LOGV()<< "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " << propertyMaxCombinedTextureImageUnits;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &propertyMaxVertexAttribs);
	CHECK_GL_ERROR_REND(this);
	ASSERT(propertyMaxVertexAttribs >= 8) << "GL_MAX_VERTEX_ATTRIBS is less than defined in OpenGL ES 2.0 standard (8): "
			<< propertyMaxVertexAttribs;
	LOGV()<< "GL_MAX_VERTEX_ATTRIBS: " << propertyMaxVertexAttribs;

	ASSERT(textureBindAllocatedEntries == nullptr) << "allocatedEntries is not nullptr";

	textureBindAllocatedEntries = new TextureBindEntry[propertyMaxCombinedTextureImageUnits];
	const GLint lastindex = propertyMaxCombinedTextureImageUnits - 1;

	TextureBindEntry& first = textureBindAllocatedEntries[0];
	first.prev = &textureBindFreeStart;
	first.next = textureBindAllocatedEntries + 1;
	first.textureUnit = GL_TEXTURE0;

	TextureBindEntry& last = textureBindAllocatedEntries[lastindex];
	last.prev = textureBindAllocatedEntries + lastindex - 1;
	last.next = &textureBindFreeEnd;
	last.textureUnit = GL_TEXTURE0 + lastindex;

	for (int i = 1; i < lastindex; ++i) {
		TextureBindEntry* b = textureBindAllocatedEntries + i;
		b->textureUnit = GL_TEXTURE0 + i;
		b->prev = b - 1;
		b->next = b + 1;
	}

	textureBindFreeStart.prev = nullptr;
	textureBindFreeStart.next = textureBindAllocatedEntries + 0;
	textureBindFreeEnd.prev = textureBindAllocatedEntries + lastindex;
	textureBindFreeEnd.next = nullptr;

	textureBindUsedStart.prev = nullptr;
	textureBindUsedStart.next = &textureBindUsedEnd;
	textureBindUsedEnd.prev = &textureBindUsedStart;
	textureBindUsedEnd.next = nullptr;

	ASSERT(vaaStates == nullptr) << "VAA statis already inited";

	vaaStates = new VertexAttribArrayState[propertyMaxVertexAttribs];

	//all is disabled here
	for (GLint i = 0; i < propertyMaxVertexAttribs; ++i) {
		vaaStates[i].index = i;
	}

	vaaEnabledStart.next = &vaaEnabledEnd;
	vaaEnabledEnd.prev = &vaaEnabledStart;

	return true;
}
void OpenGlEs20Renderer::free() {
	ASSERT(textureBindAllocatedEntries != nullptr) << "OpenGlEs20Texture is not initialized statically";

	delete[] textureBindAllocatedEntries;
	textureBindAllocatedEntries = nullptr;

	delete[] vaaStates;
	vaaStates = nullptr;

	GlGlueFunctions::free();

	Renderer::free();
}

void OpenGlEs20Renderer::clearColor(const Color& color) {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClearColor(color.r(), color.g(), color.b(), color.a());
	CHECK_GL_ERROR_REND(this);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}
void OpenGlEs20Renderer::clearDepthBuffer() {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClear(GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}

OpenGlEs20Renderer::VertexAttribArrayState* OpenGlEs20Renderer::enableVertexAttribArray(GLint index) {
	ASSERT(vaaStates != nullptr) << "ShaderProgramBase wasn't statically inited";

	VertexAttribArrayState* vaa = vaaStates + index;
	if (!vaa->enabled()) {
		glEnableVertexAttribArray(index);
		CHECK_GL_ERROR_REND(this);
	} else {
		//is enabled
		//remove from current pos
		vaa->prev->next = vaa->next;
		vaa->next->prev = vaa->prev;
	}
	//move to front
	vaa->prev = &vaaEnabledStart;
	vaa->next = vaaEnabledStart.next;
	vaaEnabledStart.next->prev = vaa;
	vaaEnabledStart.next = vaa;
	return vaa;
}

void OpenGlEs20Renderer::disableVertexAttribArrayStartingAt(VertexAttribArrayState* vaa) {
	ASSERT(vaaStates != nullptr) << "ShaderProgramBase wasn't statically inited";

	for (LinkEntry* it = vaa->next; it != &vaaEnabledEnd;) {
		LinkEntry* nextit = it->next;
		glDisableVertexAttribArray(static_cast<VertexAttribArrayState*>(it)->index);
		CHECK_GL_ERROR_REND(this);
		it->prev->next = it->next;
		it->next->prev = it->prev;
		it->prev = nullptr;
		it->next = nullptr; //redundant, enabled state is decided on prev ptr value

		it = nextit;
	}
}

void OpenGlEs20Renderer::clearColorDepthBuffer(const Color& color) {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClearColor(color.r(), color.g(), color.b(), color.a());
	CHECK_GL_ERROR_REND(this);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}

render::Texture* OpenGlEs20Renderer::createTextureImpl() {
	return new OpenGlEs20Texture { this };
}
render::VertexBuffer* OpenGlEs20Renderer::createVertexBufferImpl() {
	return new OpenGlEs20VertexBuffer { this };
}
render::IndexBuffer* OpenGlEs20Renderer::createIndexBufferImpl() {
	return new OpenGlEs20IndexBuffer { this };
}
render::RenderTarget* OpenGlEs20Renderer::createRenderTargetImpl() {
	return new OpenGlEs20FrameBuffer { this };
}
render::RenderBuffer* OpenGlEs20Renderer::createRenderBufferImpl() {
	return new OpenGlEs20RenderBuffer { this };
}

void OpenGlEs20Renderer::activateRenderTarget(RenderTargetStackEntry* target) {
	Renderer::activateRenderTarget(target);
	cullFrontFaceState.postCommand();
}

void OpenGlEs20Renderer::setDepthTestImpl(bool enabled) {
	if (enabled) {
		glEnable(GL_DEPTH_TEST);
		CHECK_GL_ERROR_REND(this);
	} else {
		glDisable(GL_DEPTH_TEST);
		CHECK_GL_ERROR_REND(this);
	}
}
void OpenGlEs20Renderer::setFaceCullingImpl(bool enabled) {
	if (enabled) {
		glEnable(GL_CULL_FACE);
		CHECK_GL_ERROR_REND(this);
	} else {
		glDisable(GL_CULL_FACE);
		CHECK_GL_ERROR_REND(this);
	}
}
void OpenGlEs20Renderer::setCullToFrontFaceImpl(bool isFront) {
	auto* target = getRenderTarget();
	//if not rendering to window (to texture), flip the front face
	bool cullFaceFlipped = target != nullptr && target->type != RENDER_TARGET_TYPE_WINDOW;
	if (isFront) {
		glCullFace(cullFaceFlipped ? GL_BACK : GL_FRONT);
		CHECK_GL_ERROR_REND(this);
	} else {
		glCullFace(cullFaceFlipped ? GL_FRONT : GL_BACK);
		CHECK_GL_ERROR_REND(this);
	}
}
void OpenGlEs20Renderer::setViewPortImpl(const render::ViewPort& vp) {
	glViewport(vp.pos.x(), vp.pos.y(), vp.size.width(), vp.size.height());
	CHECK_GL_ERROR_REND(this);
}

template<> render::Renderer* instantiateRenderer<RenderConfig::OpenGlEs20>() {
	return new OpenGlEs20Renderer();
}

} // namespace rhfw

