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
 * OpenGl30Renderer.cpp
 *
 *  Created on: 2014.06.07.
 *      Author: sipka
 */

#include <opengl30/OpenGl30Renderer.h>
#include <opengl30/texture/OpenGl30Texture.h>
#include <opengl30/buffer/OpenGl30IndexBuffer.h>
#include <opengl30/shader/OpenGl30ShaderProgramBase.h>

#include <gen/log.h>
#include <opengl30/buffer/OpenGl30FrameBuffer.h>
#include <opengl30/buffer/OpenGl30RenderBuffer.h>
#include <opengl30/buffer/OpenGl30VertexBuffer.h>

#include <gen/renderers.h>

namespace rhfw {

OpenGl30Renderer::OpenGl30Renderer()
		: Renderer { RenderConfig::OpenGl30, TextureLoadOrder::LAST_LINE_AT_ORIGIN, TextureUvOrientation::Y_DIR_UP,
				DepthRange::RANGE_NEG_1_TO_POS_1 } {
}
OpenGl30Renderer::~OpenGl30Renderer() {
}

bool OpenGl30Renderer::load() {
	if (!Renderer::load()) {
		return false;
	}

	GlGlueFunctions::load(getRenderingContext());

	CHECK_GL_ERROR_REND(this);

	LOGI()<< "OpenGL vendor: " << glGetString(GL_VENDOR);
	LOGI()<< "OpenGL renderer: " << glGetString(GL_RENDERER);
	LOGI()<< "OpenGL version: " << glGetString(GL_VERSION);
	LOGI()<< "OpenGL shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
	LOGI()<< "OpenGL extensions: " << glGetString(GL_EXTENSIONS);

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
	ASSERT(propertyMaxCombinedTextureImageUnits >= 2)
			<< "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS is less than defined in OpenGL 3.0 standard (2): "
			<< propertyMaxCombinedTextureImageUnits;
	LOGV()<< "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: " << propertyMaxCombinedTextureImageUnits;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &propertyMaxVertexAttribs);
	CHECK_GL_ERROR_REND(this);
	ASSERT(propertyMaxVertexAttribs >= 16) << "GL_MAX_VERTEX_ATTRIBS is less than defined in OpenGL 3.0 standard (16): "
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

	ASSERT(vaaStates == nullptr) << "VAA states already inited";

	vaaStates = new VertexAttribArrayState[propertyMaxVertexAttribs];

	//all is disabled here
	for (GLint i = 0; i < propertyMaxVertexAttribs; ++i) {
		vaaStates[i].index = i;
	}

	vaaEnabledStart.next = &vaaEnabledEnd;
	vaaEnabledEnd.prev = &vaaEnabledStart;

	return true;
}
void OpenGl30Renderer::free() {
	ASSERT(textureBindAllocatedEntries != nullptr) << "OpenGl30Texture is not initialized statically";

	delete[] textureBindAllocatedEntries;
	textureBindAllocatedEntries = nullptr;

	delete[] vaaStates;
	vaaStates = nullptr;

	GlGlueFunctions::free();

	Renderer::free();
}
bool OpenGl30Renderer::reload() {
	for (GLint i = 0; i < propertyMaxVertexAttribs; ++i) {
		if (vaaStates[i].enabled()) {
			glDisableVertexAttribArray(i);
			CHECK_GL_ERROR_REND(this);
		}
		vaaStates[i].prev = nullptr;
		vaaStates[i].next = nullptr;
	}

	vaaEnabledStart.next = &vaaEnabledEnd;
	vaaEnabledEnd.prev = &vaaEnabledStart;

	Renderer::reload();

	glEnable(GL_BLEND);
	CHECK_GL_ERROR_REND(this);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	CHECK_GL_ERROR_REND(this);

	return true;
}

void OpenGl30Renderer::clearColor(const Color& color) {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClearColor(color.r(), color.g(), color.b(), color.a());
	CHECK_GL_ERROR_REND(this);
	glClear(GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}
void OpenGl30Renderer::clearDepthBuffer() {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClear(GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}

OpenGl30Renderer::VertexAttribArrayState* OpenGl30Renderer::enableVertexAttribArray(GLint index) {
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

void OpenGl30Renderer::disableVertexAttribArrayStartingAt(VertexAttribArrayState* vaa) {
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

void OpenGl30Renderer::clearColorDepthBuffer(const Color& color) {
	RENDERER_CHECK_IS_DRAW_INITED();

	glClearColor(color.r(), color.g(), color.b(), color.a());
	CHECK_GL_ERROR_REND(this);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERROR_REND(this);
}

render::Texture* OpenGl30Renderer::createTextureImpl() {
	return new OpenGl30Texture { this };
}
render::VertexBuffer* OpenGl30Renderer::createVertexBufferImpl() {
	return new OpenGl30VertexBuffer { this };
}
render::IndexBuffer* OpenGl30Renderer::createIndexBufferImpl() {
	return new OpenGl30IndexBuffer { this };
}
render::RenderTarget* OpenGl30Renderer::createRenderTargetImpl() {
	return new OpenGl30FrameBuffer { this };
}
render::RenderBuffer* OpenGl30Renderer::createRenderBufferImpl() {
	return new OpenGl30RenderBuffer { this };
}

void OpenGl30Renderer::activateRenderTarget(RenderTargetStackEntry* target) {
	Renderer::activateRenderTarget(target);
	cullFrontFaceState.postCommand();
}

void OpenGl30Renderer::setDepthTestImpl(bool enabled) {
	if (enabled) {
		glEnable(GL_DEPTH_TEST);
		CHECK_GL_ERROR_REND(this);
	} else {
		glDisable(GL_DEPTH_TEST);
		CHECK_GL_ERROR_REND(this);
	}
}
void OpenGl30Renderer::setFaceCullingImpl(bool enabled) {
	if (enabled) {
		glEnable(GL_CULL_FACE);
		CHECK_GL_ERROR_REND(this);
	} else {
		glDisable(GL_CULL_FACE);
		CHECK_GL_ERROR_REND(this);
	}
}
void OpenGl30Renderer::setCullToFrontFaceImpl(bool isFront) {
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
void OpenGl30Renderer::setViewPortImpl(const render::ViewPort& vp) {
	glViewport(vp.pos.x(), vp.pos.y(), vp.size.width(), vp.size.height());
	CHECK_GL_ERROR_REND(this);
}

template<> render::Renderer* instantiateRenderer<RenderConfig::OpenGl30>() {
	return new OpenGl30Renderer { };
}

} // namespace rhfw

