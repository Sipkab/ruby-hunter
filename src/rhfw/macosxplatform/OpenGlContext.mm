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
 * OpenGlContext.cpp
 *
 *  Created on: 2016. aug. 30.
 *      Author: sipka
 */

#include <framework/render/RenderingContext.h>
#include <framework/render/RenderTarget.h>

#include <macosxplatform/MacOsxNativeWindow.h>

#include <opengl30/OpenGl30Renderer.h>

#import <AppKit/NSOpenGL.h>

#include <gen/renderers.h>

namespace rhfw {

namespace render {

class Renderer;
class OpenGlContext;

class MacOsxNativeWindowRenderSurface final : public core::WindowSizeListener, public render::RenderTarget {
private:
	OpenGlContext* context;
	core::Window* window;
protected:
	virtual bool load() override{
		window->sizeListeners += *this;
		return true;
	}
	virtual void free() override;
public:
	MacOsxNativeWindowRenderSurface(OpenGlContext* context, core::Window* window)
			: context { context }, window { window } {
		load();
	}
	~MacOsxNativeWindowRenderSurface() {
		free();
	}
	
	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override;

	virtual void bindForDrawing() override;
	virtual void finishDrawing() override;
	virtual ViewPort getDefaultViewPort() override {
		auto size = window->getWindowSize();
		return ViewPort { 0, 0, size.pixelSize };
	}
};

class OpenGlContext final: public render::RenderingContext {
private:
	friend MacOsxNativeWindowRenderSurface;

	OpenGl30Renderer* renderer;
	NSOpenGLContext* context = nil;
	core::Window* currentWindow = nullptr;
	
	int maximumMultisample = -1;
protected:
	virtual bool reload() override{
		clearContext();
		createContext();
		return context != nil;
	}

	void createContext(){
		context = initContext();
		if(context != nil){
			[context makeCurrentContext];
			setVSyncOptions(this->contextOptions.vsyncOptions);
		}
	}
	void clearContext(){
		[context clearDrawable];
		[NSOpenGLContext clearCurrentContext];
		context = nil;
		currentWindow = nullptr;
	}

	NSOpenGLContext* initContext() {
		do {
			NSOpenGLPixelFormat* pxformat = createAppropriatePixelFormat();
			if (pxformat != nil) {
				NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pxformat shareContext:nil];
				if (context != nil){
					return context;
				}
			}
			if (contextOptions.multiSamplingFactor > 1) {
				contextOptions.reduceMultiSamplingFactor();
			} else {
				break;
			}
		} while(true);
		return nil;
	}

	NSOpenGLPixelFormat* createAppropriatePixelFormat(){
		NSOpenGLPixelFormatAttribute pixelFormatAttributes[10 + 5] = {
			//NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
			NSOpenGLPFAColorSize, 24,
			NSOpenGLPFAAlphaSize, 8,
			NSOpenGLPFADepthSize, 16,
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFANoRecovery,
			0
		};
		int attrslen = 9;
		if (contextOptions.multiSamplingFactor > 1) {
			pixelFormatAttributes[attrslen++] = NSOpenGLPFAMultisample;
			pixelFormatAttributes[attrslen++] = NSOpenGLPFASampleBuffers;
			pixelFormatAttributes[attrslen++] = 1;
			pixelFormatAttributes[attrslen++] = NSOpenGLPFASamples;
			pixelFormatAttributes[attrslen++] = contextOptions.multiSamplingFactor;
		}
		pixelFormatAttributes[attrslen] = 0;
		return [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
	}
public:
	OpenGlContext(OpenGl30Renderer* renderer, const render::RenderingContextOptions* options)
		: renderer(renderer) {
		if(options != nullptr){
			this->contextOptions = *options;
		}
		if (contextOptions.multiSamplingFactor == 1){
			this->contextOptions.multiSamplingFactor = 0;
		}
		createContext();
	}
	~OpenGlContext() {
		[context clearDrawable];
		[NSOpenGLContext clearCurrentContext];
	}

	virtual void attachWindow(core::Window* window) override{
		ASSERT(window != nullptr) << "Window to attach is nullptr";
		ASSERT(!window->getRenderSurface().isAttached()) << "Window is already attached";
		ASSERT(window->getNativeWindow() != nullptr) << "Native window is not available";
		
		Resource<render::MacOsxNativeWindowRenderSurface> surface{ new ResourceBlock{ new MacOsxNativeWindowRenderSurface{ this, window } } };

		window->getRenderSurface().set(renderer, surface);
	}
	virtual void detachWindow(core::Window* window) override{
		ASSERT(window->getRenderSurface().getRenderTarget() != nullptr) << "Trying to detach already detached window";
		window->getRenderSurface().reset();
	}

	virtual VSyncOptions getSupportedVSyncOptions() override{
		return VSyncOptions::VSYNC_OFF | VSyncOptions::VSYNC_ON;
	}
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) override{
		this->contextOptions.vsyncOptions = options;
		switch (options & VSyncOptions::TYPE_MASK) {
			case VSyncOptions::VSYNC_OFF:{
				GLint swapintervalvalue = 0;
				[context setValues: &swapintervalvalue forParameter: NSOpenGLCPSwapInterval];
				break;
			}
			case VSyncOptions::VSYNC_ON:{
				GLint swapintervalvalue = 1;
				[context setValues: &swapintervalvalue forParameter: NSOpenGLCPSwapInterval];
				break;
			}
			default:
				LOGW() << "Unsupported VSync option: " << options;
				return RenderingContextOptionsResult::FAILED;
		}
		return RenderingContextOptionsResult::SUCCESS;
	}

	virtual RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) override {
		RenderingContextOptionsResult result = RenderingContextOptionsResult::NO_FLAG;
		unsigned int msfactor = options.multiSamplingFactor;
		if (msfactor == 1){
			msfactor = 0;
		}
		if (msfactor != contextOptions.multiSamplingFactor) {
			//multisampling factor changed
			contextOptions.multiSamplingFactor = msfactor;
			result |= RenderingContextOptionsResult::RELOAD_REQUIRED;
		}
		if (options.vsyncOptions != contextOptions.vsyncOptions) {
			if (!HAS_FLAG(result, RenderingContextOptionsResult::RELOAD_REQUIRED)) {
				result |= setVSyncOptions(options.vsyncOptions);
			}
			//else it is going to be set on reload
		}
		return result;
	}

	unsigned int getMaximumMultiSampleFactor() override {
		if (maximumMultisample < 0){
			int test = 2;
			while(true){
				NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
					NSOpenGLPFAMultisample, 
					NSOpenGLPFASampleBuffers, 1,
					NSOpenGLPFASamples, (unsigned int)test,
					0
				};
				NSOpenGLPixelFormat* pxformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
				if(pxformat == nil){
					break;
				}
				GLint val = 0;
				[pxformat getValues: &val forAttribute: NSOpenGLPFASamples forVirtualScreen: 0];
				if(val != test){
					break;
				}
				test *= 2;
			}
			maximumMultisample = (unsigned int) test / 2;
		}
		return maximumMultisample;
	}
};
	
void MacOsxNativeWindowRenderSurface::bindForDrawing() {
	if(context->currentWindow != window){
		context->context.view = window->getNativeWindow()->window.view;
		context->currentWindow = window;
	}
	if (context->renderer->getBoundFrameBufferName() != 0) {
		context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->setBoundFrameBufferName(0);
	}
}
void MacOsxNativeWindowRenderSurface::finishDrawing() {
	[context->context flushBuffer];
}

void MacOsxNativeWindowRenderSurface::free() {
	if(context->currentWindow == window){
		context->context.view = nil;	
		context->currentWindow = nullptr;
	}
}

void MacOsxNativeWindowRenderSurface::onSizeChanged(core::Window& window, const core::WindowSize& size) {
	if(context->currentWindow == this->window){
		[context->context update];
	}
}

} // namespace render

template<> 
render::RenderingContext* instantiateRenderingContext<RenderConfig::OpenGl30>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	return new render::OpenGlContext(static_cast<OpenGl30Renderer*>(renderer), options);
}

}  // namespace rhfw
