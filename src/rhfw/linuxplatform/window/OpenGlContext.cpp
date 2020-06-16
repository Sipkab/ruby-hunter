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
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#include <framework/render/RenderingContext.h>
#include <framework/render/RenderTarget.h>
#include <framework/render/Renderer.h>
#include <framework/utils/utility.h>
#include <linuxplatform/window/appglue.h>

#include <gen/renderers.h>
#include <gen/types.h>

#include <linuxplatform/window/glxfunctions.h>
#include <linuxplatform/window/x11functions.h>

#include <linuxplatform/window/OpenGlContext.h>

#define GLX_SAMPLE_BUFFERS_ARB              100000
#define GLX_SAMPLES_ARB                     100001

#define GLX_SAMPLE_BUFFERS_SGIS         	100000
#define GLX_SAMPLES_SGIS                	100001

#define GLX_SWAP_INTERVAL_EXT               0x20F1
#define GLX_MAX_SWAP_INTERVAL_EXT           0x20F2

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

namespace rhfw {
namespace render {
class LinuxWindowRenderSurface: public render::RenderTarget {
private:
	OpenGlContext* context;
	core::Window* window;

	int swapInterval = -2;
protected:
	virtual bool load() override {
		return true;
	}
	virtual void free() override {
	}
public:
	LinuxWindowRenderSurface(OpenGlContext* context, core::Window* window)
			: context { context }, window { window } {
		load();
	}
	~LinuxWindowRenderSurface() {
		free();
	}

	virtual void bindForDrawing() override;
	virtual void finishDrawing() override;
	virtual ViewPort getDefaultViewPort() override {
		auto size = window->getWindowSize();
		return ViewPort { 0, 0, size.pixelSize };
	}

	void updateSwapInterval(int interval) {
		if (this->swapInterval == interval) {
			return;
		}
		if (context->glxfunc_glXSwapIntervalEXT != nullptr) {
			this->swapInterval = interval;

			Display* display = x11server::getMainDisplay();
			xfunc_XLockDisplay(display);

//			context->setWindowLocked(display, window);
			context->glxfunc_glXSwapIntervalEXT(display, window->getNativeWindow()->window, swapInterval);
			xfunc_XUnlockDisplay(display);
		}
	}

};

#define LOAD_CONTEXT_PROC(name) glxfunc_##name = (PROTO_##name) glxfunc_glXGetProcAddress((const GLubyte*) (STRINGIZE(name))); \
	WARN(glxfunc_##name == NULL) << "GLX method not found: " << STRINGIZE(name)

OpenGlContext::OpenGlContext(OpenGl30Renderer* renderer, const render::RenderingContextOptions* options)
		: renderer(renderer) {
	if (options != nullptr) {
		contextOptions = *options;
	}

	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);

	if (glxfunc_glXQueryExtensionsString == nullptr) {
		glxextensions = "";
	} else {
		glxextensions = glxfunc_glXQueryExtensionsString(display, xfunc_XDefaultScreen(display));
		if (glxextensions == nullptr) {
			glxextensions = "";
		}
	}
	LOGI() << "GLX extensions: " << glxextensions;
	hasMultisampleExtensions = hasExtensionString(glxextensions, "GLX_ARB_multisample")
			|| hasExtensionString(glxextensions, "GLX_SGIS_multisample");
	if (!hasMultisampleExtensions) {
		maximumMultisample = 0;
	}
	if (contextOptions.multiSamplingFactor == 1 || (contextOptions.multiSamplingFactor > 1 && !hasMultisampleExtensions)) {
		contextOptions.multiSamplingFactor = 0;
	}

	//GLX functions are context independent as in GLX_ARB_get_proc_address
	if (hasExtensionString(glxextensions, "GLX_EXT_swap_control")) {
		LOAD_CONTEXT_PROC(glXSwapIntervalEXT);
		supportedVSyncOptions = VSyncOptions::VSYNC_ON | VSyncOptions::VSYNC_OFF;
	} else {
		supportedVSyncOptions = VSyncOptions::NO_FLAG;
	}
	hasExtensionGLX_EXT_swap_control_tear = hasExtensionString(glxextensions, "GLX_EXT_swap_control_tear");

	createAppropriateContext(display);

	glxfunc_glXMakeCurrent(display, dummyGlxPixmap, context);
	XDEBUGSYNC(display);

	xfunc_XUnlockDisplay(display);
}

OpenGlContext::~OpenGlContext() {
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);

	glxfunc_glXMakeCurrent(display, None, NULL);
	glxfunc_glXDestroyContext(display, context);
	glxfunc_glXDestroyGLXPixmap(display, dummyGlxPixmap);
	xfunc_XFreePixmap(display, dummyPixmap);
	XDEBUGSYNC(display);

	visualInfo.releaseLocked();

	xfunc_XUnlockDisplay(display);
}

x11server::VisualInfoTracker OpenGlContext::getAppropriateVisual(Display* display, const RenderingContextOptions& options,
		GLXFBConfig* outfbconfig) {
	//TODO if GLX version < 1.3 we cant use glXChooseFBConfig
	//assume its already locked
	int attributes[11 * 2 + 2 * 2 + 1] = { //
			GLX_X_RENDERABLE, True, //
					GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, //
					GLX_RENDER_TYPE, GLX_RGBA_BIT, //
					GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, //
					GLX_RED_SIZE, 8, //
					GLX_GREEN_SIZE, 8, //
					GLX_BLUE_SIZE, 8, //
					GLX_ALPHA_SIZE, 8, //
					GLX_DEPTH_SIZE, 24, //
					GLX_STENCIL_SIZE, 8, //
					GLX_DOUBLEBUFFER, True, //
			};
	int attrslen = 11 * 2;
	if (options.multiSamplingFactor > 1) {
		attributes[attrslen++] = GLX_SAMPLE_BUFFERS_ARB;
		attributes[attrslen++] = 1;
		attributes[attrslen++] = GLX_SAMPLES_ARB;
		attributes[attrslen++] = options.multiSamplingFactor;
	}
	attributes[attrslen] = None;

	GLXFBConfig fbconfig = 0;
	int fbcount;
	GLXFBConfig* fbc = glxfunc_glXChooseFBConfig(display, 0, attributes, &fbcount);
	if (fbc == nullptr) {
		return nullptr;
	}
	if (fbcount == 0) {
		xfunc_XFree(fbc);
		return nullptr;
	}
	fbconfig = fbc[0];
	xfunc_XFree(fbc);

	XVisualInfo* visual = glxfunc_glXGetVisualFromFBConfig(display, fbconfig);

	if (visual == nullptr) {
		LOGW() << "Got nullptr visual from glXGetVisualFromFBConfig";
		return nullptr;
	}
	*outfbconfig = fbconfig;
	XDEBUGSYNC(display);
	return x11server::VisualInfoTracker::make(visual);
}

bool OpenGlContext::createAppropriateContext(Display* display) {
	while (true) {
		GLXFBConfig fbconfig;
		auto vis = getAppropriateVisual(display, contextOptions, &fbconfig);
		if (vis != nullptr) {
			Pixmap pxmap = xfunc_XCreatePixmap(display, xfunc_XDefaultRootWindow(display), 4, 4, vis->depth);
			XDEBUGSYNC(display);
			if (pxmap) {
				GLXPixmap glxpixmap = glxfunc_glXCreateGLXPixmap(display, vis, pxmap);
				XDEBUGSYNC(display);
				if (glxpixmap) {
					GLXContext context = glxfunc_glXCreateContext(display, vis, NULL, GL_TRUE);
					XDEBUGSYNC(display);
					if (context) {
						this->context = context;
						this->dummyPixmap = pxmap;
						this->dummyGlxPixmap = glxpixmap;
						this->visualInfo = util::move(vis);
						break;
					} else {
						glxfunc_glXDestroyGLXPixmap(display, glxpixmap);
						xfunc_XFreePixmap(display, pxmap);
						XDEBUGSYNC(display);
					}
				} else {
					xfunc_XFreePixmap(display, pxmap);
					XDEBUGSYNC(display);
				}
			}
		}
		if (contextOptions.multiSamplingFactor <= 1) {
			THROW() << "Failed to create OpenGL context";
			//failed to create context;
			return false;
		}
		contextOptions.reduceMultiSamplingFactor();
	}

	if (glxfunc_glXSwapIntervalEXT != nullptr) {

		supportedVSyncOptions = VSyncOptions::VSYNC_ON | VSyncOptions::VSYNC_OFF;

		if (hasExtensionGLX_EXT_swap_control_tear) {
			supportedVSyncOptions |= VSyncOptions::ADAPTIVE;
		}

		if (contextOptions.vsyncOptions != VSyncOptions::NO_FLAG) {
			setVSyncOptions(contextOptions.vsyncOptions);
		}
	}

	return true;
}

void OpenGlContext::attachWindow(core::Window* window) {
	ASSERT(window != nullptr) << "Window to attach is nullptr";
	ASSERT(!window->getRenderSurface().isAttached()) << "Window is already attached";
	ASSERT(window->getNativeWindow() != nullptr) << "Native window is not available";

	if (window->getNativeWindow()->visualInfo->visual != visualInfo->visual) {
		LOGI() << "OpenGL context visuals doesn't match with window";
		window->recreateWindow(visualInfo);
	} else {

	}
	Resource<render::LinuxWindowRenderSurface> surface { new ResourceBlock(new LinuxWindowRenderSurface(this, window)) };

	window->getRenderSurface().set(renderer, surface);

	surface->updateSwapInterval(swapInterval);
}
void OpenGlContext::detachWindow(core::Window* window) {
	ASSERT(window->getRenderSurface().getRenderTarget() != nullptr) << "Trying to detach already detached window";
	if (currentWindow == window) {
		setWindow(nullptr);
	}
	window->getRenderSurface().reset();
}

void OpenGlContext::setWindow(core::Window* window) {
	if (currentWindow == window) {
		return;
	}
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	if (window != nullptr) {
		glxfunc_glXMakeCurrent(display, window->getNativeWindow()->window, context);
	} else {
		glxfunc_glXMakeCurrent(display, dummyGlxPixmap, context);
	}

	XDEBUGSYNC(display);
	currentWindow = window;
	xfunc_XUnlockDisplay(display);
}
void OpenGlContext::setWindowLocked(Display* display, core::Window* window) {
	if (currentWindow == window) {
		return;
	}
	if (window != nullptr) {
		glxfunc_glXMakeCurrent(display, window->getNativeWindow()->window, context);
	} else {
		glxfunc_glXMakeCurrent(display, dummyGlxPixmap, context);
	}

	XDEBUGSYNC(display);
	currentWindow = window;
}

VSyncOptions OpenGlContext::getSupportedVSyncOptions() {
	return supportedVSyncOptions;
}
RenderingContextOptionsResult OpenGlContext::setVSyncOptions(VSyncOptions options) {
	switch (options & VSyncOptions::TYPE_MASK) {
		case VSyncOptions::VSYNC_OFF: {
			swapInterval = 0;
			break;
		}
		case VSyncOptions::VSYNC_ON: {
			swapInterval = 1;
			break;
		}
		case VSyncOptions::SWAPINTERVAL: {
			swapInterval = (int) ((options & VSyncOptions::SWAPINTERVAL_MASK) >> VSyncOptions::SWAPINTERVAL_SHIFT);
			break;
		}
		case VSyncOptions::ADAPTIVE: {
			ASSERT(HAS_FLAG(supportedVSyncOptions, VSyncOptions::ADAPTIVE));
			if (!HAS_FLAG(supportedVSyncOptions, VSyncOptions::ADAPTIVE)) {
				return setVSyncOptions(VSyncOptions::VSYNC_ON);
			}
			swapInterval = -1;
			break;
		}
		default: {
			LOGW() << "Unsupported VSync option: " << options;
			return RenderingContextOptionsResult::FAILED;
		}
	}
	//TODO change for attached windows
	for (auto& w : renderer->getAttachedWindows().objects()) {
		if (w.getRenderSurface().isAttached()) {
			auto& surface = static_cast<LinuxWindowRenderSurface&>(*w.getRenderSurface().getRenderTarget());
			surface.updateSwapInterval(swapInterval);
		}
	}
	this->contextOptions.vsyncOptions = options;
	return RenderingContextOptionsResult::SUCCESS;
}

unsigned int OpenGlContext::getMaximumMultiSampleFactor() {
	if (maximumMultisample < 0) {
		Display* display = x11server::getMainDisplay();
		xfunc_XLockDisplay(display);
		int test = 2;
		while (true) {
			int attributes[] = { //
					GLX_SAMPLE_BUFFERS_ARB, 1, //
							GLX_SAMPLES_ARB, test, //
							None };

			int fbcount;
			GLXFBConfig *fbc = glxfunc_glXChooseFBConfig(display, 0, attributes, &fbcount);
			if (fbc == nullptr) {
				break;
			}
			if (fbcount == 0) {
				xfunc_XFree(fbc);
				break;
			}
			GLXFBConfig fbconfig = fbc[0];
			xfunc_XFree(fbc);
			XVisualInfo* visual = glxfunc_glXGetVisualFromFBConfig(display, fbconfig);

			if (visual == nullptr) {
				LOGW() << "Got nullptr visual from glXGetVisualFromFBConfig";
				break;
			}
			xfunc_XFree(visual);

			test *= 2;
		}
		xfunc_XUnlockDisplay(display);

		maximumMultisample = (unsigned int) test / 2;
	}
	return maximumMultisample;
}

RenderingContextOptionsResult OpenGlContext::setRenderingContextOptions(const RenderingContextOptions& options) {
	RenderingContextOptionsResult result = RenderingContextOptionsResult::NO_FLAG;
	unsigned int msfactor = options.multiSamplingFactor;
	if (msfactor == 1) {
		msfactor = 0;
	}
	if (msfactor != contextOptions.multiSamplingFactor) {
		//multisampling factor changed
		//only change if we have the appropriate extension
		if (hasMultisampleExtensions) {
			contextOptions.multiSamplingFactor = msfactor;
			result |= RenderingContextOptionsResult::RELOAD_REQUIRED;
		} else {
			if (options.multiSamplingFactor > 1) {
				result |= RenderingContextOptionsResult::FAILED;
			} else {
				result |= RenderingContextOptionsResult::SUCCESS;
			}
		}
	}

	if (options.vsyncOptions != contextOptions.vsyncOptions) {
		if (!HAS_FLAG(result, RenderingContextOptionsResult::RELOAD_REQUIRED)) {
			result |= setVSyncOptions(options.vsyncOptions);
		}
		//else it is going to be set on reload
	}

	return result;
}

bool OpenGlContext::reload() {
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);

	currentWindow = nullptr;

	glxfunc_glXMakeCurrent(display, None, NULL);
	glxfunc_glXDestroyContext(display, context);
	glxfunc_glXDestroyGLXPixmap(display, dummyGlxPixmap);
	xfunc_XFreePixmap(display, dummyPixmap);
	XDEBUGSYNC(display);

	visualInfo.releaseLocked();

	bool res = createAppropriateContext(display);

	if (res) {
		glxfunc_glXMakeCurrent(display, dummyGlxPixmap, context);
		XDEBUGSYNC(display);
	}

	xfunc_XUnlockDisplay(display);

	return res;
}

void LinuxWindowRenderSurface::bindForDrawing() {
	context->setWindow(window);
	if (context->renderer->getBoundFrameBufferName() != 0) {
		context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL_ERROR_REND(context->renderer);
		context->renderer->setBoundFrameBufferName(0);
	}
}
void LinuxWindowRenderSurface::finishDrawing() {
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	glxfunc_glXSwapBuffers(display, window->getNativeWindow()->window);
	xfunc_XUnlockDisplay(display);
	XDEBUGSYNC(display);
}

} // namespace render

template<> render::RenderingContext* instantiateRenderingContext<RenderConfig::OpenGl30>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	return new render::OpenGlContext(static_cast<OpenGl30Renderer*>(renderer), options);
}

}  // namespace rhfw
