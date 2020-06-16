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
 * AndroidOpenGlEs20EglContext.cpp
 *
 *  Created on: 2016. maj. 28.
 *      Author: sipka
 */

#include <gen/renderers.h>

#include <framework/render/RenderingContext.h>
#include <framework/render/RenderTarget.h>
#include <opengles20/OpenGlEs20Renderer.h>

#include <gen/configuration.h>

#include <EGL/egl.h>

#include <gen/log.h>

#include <dlfcn.h>

#if LOGGING_ENABLED
//use if(true) for not to break eclipse macro formatting
#define CHECK_EGL_ERROR()  \
	if(true){ \
		EGLint _egl_error = EGL_SUCCESS; \
		if((_egl_error = eglGetError()) != EGL_SUCCESS) { \
			THROW() << "EGL has error state: " << _egl_error; \
		} \
	}
#else
#define CHECK_EGL_ERROR()
#endif

namespace rhfw {
namespace render {

class AndroidOpenGlEs20EglContext;

class AndroidOpenGlEs20RendererSurface final : public render::RenderTarget {
private:
	AndroidOpenGlEs20EglContext* context;
	core::Window* window;
protected:
	virtual bool load() override;
	virtual void free() override;
public:
	EGLSurface surface = EGL_NO_SURFACE;

	AndroidOpenGlEs20RendererSurface(AndroidOpenGlEs20EglContext* context, core::Window* window)
			: context { context }, window { window } {
		load();
	}
	~AndroidOpenGlEs20RendererSurface() {
		free();
	}

	virtual void bindForDrawing() override;
	virtual void finishDrawing() override;
	virtual ViewPort getDefaultViewPort() override {
		auto size = window->getWindowSize();
		return ViewPort { 0, 0, size.pixelSize };
	}
};

class AndroidOpenGlEs20EglContext: public render::RenderingContext {
private:
	friend AndroidOpenGlEs20RendererSurface;
	OpenGlEs20Renderer* renderer;

	EGLDisplay display;
	EGLContext context = EGL_NO_CONTEXT;

	EGLConfig config;

	EGLSurface dummysurface = EGL_NO_SURFACE;

	EGLSurface currentsurface = EGL_NO_SURFACE;

	EGLint contextMajorVersion;
	EGLint contextMinorVersion;

public:
	AndroidOpenGlEs20EglContext(OpenGlEs20Renderer* renderer, EGLDisplay display, EGLContext context, EGLConfig config, EGLint contextmajor,
			EGLint contextminor)
			: renderer(renderer), display(display), context(context), config(config), contextMajorVersion(contextmajor), contextMinorVersion(
					contextminor) {
		LOGI()<< "EGLContext with OpenGL ES version: " << contextmajor << "." << contextminor;

		const EGLint dummyattr[] = {
			EGL_WIDTH, 1,
			EGL_HEIGHT, 1,
			EGL_NONE};
		dummysurface = eglCreatePbufferSurface(display, config, dummyattr);
		CHECK_EGL_ERROR();
		ASSERT(dummysurface != EGL_NO_SURFACE) << "eglCreatePbufferSurface failed";

		setSurface(dummysurface);
	}
	virtual ~AndroidOpenGlEs20EglContext() {
		if (display != EGL_NO_DISPLAY) {
			EGLBoolean result;

			result = eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			CHECK_EGL_ERROR();
			ASSERT(result != EGL_FALSE) << "eglMakeCurrent failed";

			if (dummysurface != EGL_NO_SURFACE) {
				result = eglDestroySurface(display, dummysurface);
				CHECK_EGL_ERROR();
				ASSERT(result != EGL_FALSE) << "eglDestroySurface failed";

				dummysurface = EGL_NO_SURFACE;
			}
			if (context != EGL_NO_CONTEXT) {
				result = eglDestroyContext(display, context);
				CHECK_EGL_ERROR();
				ASSERT(result != EGL_FALSE) << "eglDestroyContext failed";

				context = EGL_NO_CONTEXT;
			}
			result = eglTerminate(display);
			CHECK_EGL_ERROR();
			ASSERT(result != EGL_FALSE) << "eglTerminate failed";

			display = EGL_NO_DISPLAY;
			LOGI()<< "Destroyed AndroidOpenGlEs20EglContext " << this;
		}
	}
	virtual void attachWindow(core::Window* window) override {
		ASSERT(window != nullptr) << "Window to attach is nullptr";
		ASSERT(!window->getRenderSurface().isAttached()) << "Window is already attached";
		ASSERT(window->getNativeWindow() != nullptr) << "Android native window is not available";

		Resource<render::AndroidOpenGlEs20RendererSurface> surface {new ResourceBlock(new AndroidOpenGlEs20RendererSurface(this, window))};

		window->getRenderSurface().set(renderer, surface);
	}
	virtual void detachWindow(core::Window* window) override {
		ASSERT(window->getRenderSurface().getRenderTarget() != nullptr) << "Trying to detach already detached window";
		window->getRenderSurface().reset();
	}

	void clearIfCurrentSurface(EGLSurface surface) {
		if (currentsurface != surface) {
			return;
		}

		setSurface(dummysurface);
	}
	void setSurface(EGLSurface surface) {
		ASSERT(surface != EGL_NO_SURFACE || dummysurface == EGL_NO_SURFACE) << "Trying to set EGL_NO_SURFACE for current surface";
		if (currentsurface == surface) {
			return;
		}

		EGLBoolean result;
		if (surface == EGL_NO_SURFACE) {
			LOGW() << "No dummy EGL surface available";
			result = eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		} else {
			result = eglMakeCurrent(display, surface, surface, context);
		}
		CHECK_EGL_ERROR();
		ASSERT(result != EGL_FALSE) << "eglMakeCurrent failed for surface: " << surface;
		currentsurface = surface;
	}
	void swapSurface(EGLSurface surface) {
		ASSERT(surface != EGL_NO_SURFACE) << "surface is EGL_NO_SURFACE";
		EGLBoolean result = eglSwapBuffers(display, surface);
		CHECK_EGL_ERROR();
		ASSERT(result != EGL_FALSE) << "eglSwapBuffers failed for surface: " << surface;
		//if (result != EGL_TRUE) {
		//possible error scenarios:
		// - activity started, device goes to sleep, awaken from sleep, swapbuffers fail (only the first time in activity lifetime).
		//            WINDOW_DESTROYING is not yet received.
		// - on Note 3, after fresh install, the activity is recreated after like 5 sec. WINDOW_DESTROYING is not yet received.

		//clear error values, we will receive WINDOW_DESTROYING event soon
		//EGLint err = eglGetError();
		//GLenum glerr = glGetError();

		//LOGE("eglSwapBuffers failed with EGL error: 0x%X, GL error: 0x%X, for surface: %p", err, glerr, surface);
		//}
	}
};

bool AndroidOpenGlEs20RendererSurface::load() {
	surface = eglCreateWindowSurface(context->display, context->config, window->getNativeWindow(), nullptr);
	CHECK_EGL_ERROR();
	ASSERT(surface != EGL_NO_SURFACE) << "eglCreateWindowSurface failed";

	return true;
}
void AndroidOpenGlEs20RendererSurface::free() {
	context->clearIfCurrentSurface(surface);

	EGLBoolean result;
	result = eglDestroySurface(context->display, surface);
	CHECK_EGL_ERROR();
	ASSERT(result != EGL_FALSE) << "eglDestroySurface failed";
}

void AndroidOpenGlEs20RendererSurface::bindForDrawing() {
	context->setSurface(surface);
	if (context->renderer->getBoundFrameBufferName() != 0) {
		context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->setBoundFrameBufferName(0);
	}
}
void AndroidOpenGlEs20RendererSurface::finishDrawing() {
	context->swapSurface(surface);
}

}  // namespace render
}  // namespace rhfw

namespace rhfw {

static bool hasExtensionString(const char* extensions, const char* ext, unsigned int extlen) {
	do {
		const char* pos = strstr(extensions, ext);
		if (pos == NULL) {
			return false;
		}
		if (pos[extlen] == 0 || pos[extlen] == ' ') {
			return true;
		}
		//found, but ext is prefix of some other
		extensions = pos + extlen;
	} while (true);

	return false;
}
template<size_t N>
static bool hasExtensionString(const char* extensions, const char (&ext)[N]) {
	return hasExtensionString(extensions, ext, N - 1);
}

/**
 * From EGL_KHR_create_context
 */
#define EGL_CONTEXT_MAJOR_VERSION_KHR			    EGL_CONTEXT_CLIENT_VERSION
#define EGL_CONTEXT_MINOR_VERSION_KHR			    0x30FB
#define EGL_OPENGL_ES3_BIT_KHR					    0x00000040

template<>
render::RenderingContext* instantiateRenderingContext<RenderConfig::OpenGlEs20>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	CHECK_EGL_ERROR();
	ASSERT(display != EGL_NO_DISPLAY) << "eglGetDisplay failed";
	if (display == EGL_NO_DISPLAY) {
		return nullptr;
	}

	EGLint major;
	EGLint minor;
	EGLBoolean result;
	result = eglInitialize(display, &major, &minor);
	CHECK_EGL_ERROR();
	ASSERT(result == EGL_TRUE) << "eglInitialize failed: " << result;
	if (result != EGL_TRUE) {
		return nullptr;
	}
	LOGI()<< "EGL version: " << major << "." << minor;

	EGLContext context = EGL_NO_CONTEXT;
	EGLConfig config;
	EGLint numConfigs;

	EGLint contextmajor = 3;
	EGLint contextminor = 1;

	const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
	CHECK_EGL_ERROR();
	if (extensions == nullptr) {
		LOGE()<< "Failed to query EGL extensions";
		extensions = "";
	}
	LOGI()<< "EGL extensions: " << extensions;
	if (hasExtensionString(extensions, "EGL_KHR_create_context")) {
		//can create OpenGL ES 3.x context
		const EGLint configattribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_ALPHA_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_NONE };

		result = eglChooseConfig(display, configattribs, &config, 1, &numConfigs);
		if (result && numConfigs > 0) {
			//proceed with context creation
			//try 3.1
			EGLint contextattribs[] = {
			EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
			EGL_CONTEXT_MINOR_VERSION_KHR, 1,
			EGL_NONE };

			context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextattribs);
			if (context == EGL_NO_CONTEXT) {
				LOGW()<< "Failed to create OpenGL ES 3.1 context";
				contextattribs[3] = 0;

				//try 3.0
				context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextattribs);
				if (context == EGL_NO_CONTEXT) {
					LOGW()<< "Failed to create OpenGL ES 3.0 context";
				} else {
					contextmajor = 3;
					contextminor = 0;
					goto label_context_created;
				}
			} else {
				contextmajor = 3;
				contextminor = 1;
				goto label_context_created;
			}
		}
	}

	{
		//fallback to OpenGL ES 2.0 context
		const EGLint configattribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_ALPHA_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_NONE };
		const EGLint contextattribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE };

		result = eglChooseConfig(display, configattribs, &config, 1, &numConfigs);
		ASSERT(result == EGL_TRUE) << "eglChooseConfig failed: " << result << " EGL error: " << eglGetError();
		if (!result) {
			return nullptr;
		}

		context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextattribs);
		ASSERT(context != EGL_NO_CONTEXT) << "eglCreateContext failed" << " EGL error: " << eglGetError();
		if (context == EGL_NO_CONTEXT) {
			return nullptr;
		}

		contextmajor = 2;
		contextminor = 0;
	}
	label_context_created:

	return new render::AndroidOpenGlEs20EglContext(static_cast<OpenGlEs20Renderer*>(renderer), display, context, config, contextmajor,
			contextminor);
}

}  // namespace rhfw
