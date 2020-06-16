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
//
//  EglContext.mm
//  TestApp
//
//  Created by User on 2016. 03. 12..
//  Copyright © 2016. User. All rights reserved.
//
#include <opengles20/OpenGlEs20Renderer.h>
#include <framework/render/Renderer.h>
#include <framework/core/Window.h>
#include <framework/render/RenderTarget.h>
#include <framework/render/RenderingContext.h>

#include <gen/log.h>
#include <gen/renderers.h>

#import <iosplatform/AppDelegate.h>

namespace rhfw {
namespace render {

class Renderer;

class IosOpenGlEs20EglContext: public render::RenderingContext {
private:
public:
	OpenGlEs20Renderer* renderer;
	EAGLContext* eaglcontext;

	IosOpenGlEs20EglContext(OpenGlEs20Renderer* renderer)
		: renderer(renderer) {
		eaglcontext = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES3];
		if (eaglcontext == nil) {
			eaglcontext = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES2];
		}
		[EAGLContext setCurrentContext: eaglcontext];
	}
	~IosOpenGlEs20EglContext(){
	}

	virtual void attachWindow(core::Window* window) override;
	virtual void detachWindow(core::Window* window) override;

};

class IosOpenGlEs20RendererSurface : public core::WindowSizeListener, public render::RenderTarget{
private:
	IosOpenGlEs20EglContext* context;
	
	core::Window* window;
	
	GLuint colorrenderbuf = 0;
	GLuint depthrenderbuf = 0;
	GLuint framebuffer = 0;
protected:
	virtual bool load() override {
		context->renderer->glGenRenderbuffers(1, &colorrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glGenRenderbuffers(1, &depthrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glGenFramebuffers(1, &framebuffer);
		CHECK_GL_ERROR_REND(context->renderer);
		
		window->sizeListeners += *this;
		return true;
	}
	virtual void free() override {
		context->renderer->glDeleteFramebuffers(1, &framebuffer);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glDeleteRenderbuffers(1, &colorrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glDeleteRenderbuffers(1, &depthrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
	}
public:
	IosOpenGlEs20RendererSurface(IosOpenGlEs20EglContext* context, core::Window* window) : context { context }, window { window } {
		load();
	}
	~IosOpenGlEs20RendererSurface(){
		free();
	}
	
	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override {
		context->renderer->glBindRenderbuffer(GL_RENDERBUFFER, colorrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		[context->eaglcontext renderbufferStorage:GL_RENDERBUFFER fromDrawable:window.getNativeLayer()->layer];
		CHECK_GL_ERROR_REND(context->renderer);
		
		context->renderer->glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.pixelSize.width(), size.pixelSize.height());
		CHECK_GL_ERROR_REND(context->renderer);
		
		context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		
		context->renderer->setBoundFrameBufferName(framebuffer);
		
		/*
		GLenum error = context->renderer->glGetError();
		GLenum comp = context->renderer->glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ASSERT(comp == GL_FRAMEBUFFER_COMPLETE && error == GL_NO_ERROR) << "Failed to create framebuffer " << comp << " - " << error;
		*/
	}
	
	virtual void bindForDrawing() override{
		context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		CHECK_GL_ERROR_REND(context->renderer);
		
		context->renderer->setBoundFrameBufferName(framebuffer);
	}
	virtual void finishDrawing() override{
		context->renderer->glBindRenderbuffer(GL_RENDERBUFFER, colorrenderbuf);
		CHECK_GL_ERROR_REND(context->renderer);
		BOOL res = [context->eaglcontext presentRenderbuffer:GL_RENDERBUFFER];
		CHECK_GL_ERROR_REND(context->renderer);
		ASSERT(res == YES) << "Failed to present renderbuffer";
	}

	virtual ViewPort getDefaultViewPort() override {
		auto size = window->getWindowSize();
		return ViewPort { 0, 0, size.pixelSize.width(), size.pixelSize.height() };
	}
};

void IosOpenGlEs20EglContext::attachWindow(core::Window* window) {
	Resource<render::IosOpenGlEs20RendererSurface> surface{ new ResourceBlock(new render::IosOpenGlEs20RendererSurface(this, window)) };
	window->getRenderSurface().set(renderer, surface);
}
void IosOpenGlEs20EglContext::detachWindow(core::Window* window) {
	window->getRenderSurface().reset();
}
} // namespace render

template<> 
render::RenderingContext* instantiateRenderingContext<RenderConfig::OpenGlEs20>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	ASSERT(renderer->getRendererType() == RenderConfig::OpenGlEs20) << renderer->getRendererType();
	return new render::IosOpenGlEs20EglContext(static_cast<OpenGlEs20Renderer*>(renderer));
}

} // namespace rhfw