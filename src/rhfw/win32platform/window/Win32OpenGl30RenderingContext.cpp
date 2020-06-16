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
#undef NOGDI
#include <windows.h>

#include <win32platform/window/Win32OpenGl30RenderingContext.h>
#include <opengl30/OpenGl30Renderer.h>
#include <framework/core/Window.h>
#include <win32platform/window/Win32Platform.h>
#include <win32platform/window/WindowHandle.h>

#include <gen/renderers.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB			0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	0x00000002

#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

#define WGL_SAMPLE_BUFFERS_ARB               0x2041
#define WGL_SAMPLES_ARB                      0x2042

#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024

#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

#define CHECK_SYSCALL_ERROR() ASSERT(res) << GetLastError()

#define DUMMY_WINDOW_CLASS_NAME "OpenGLContextDummyWindow"

static PIXELFORMATDESCRIPTOR basicPixelFormatDescriptor = { //
		sizeof(PIXELFORMATDESCRIPTOR), 		// size of this pfd
				1, 							// version number
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
				PFD_TYPE_RGBA,            	//The kind of framebuffer. RGBA or palette.
				32,                        	//Colordepth of the framebuffer.
				0, 0, 0, 0, 0, 0, 			// color bits ignored
				0, 							// no alpha buffer
				0, 							// shift bit ignored
				0, 							// no accumulation buffer
				0, 0, 0, 0, 				// accum bits ignored
				24,                        	//Number of bits for the depthbuffer
				8,                        	//Number of bits for the stencilbuffer
				0,                        	//Number of Aux buffers in the framebuffer.
				PFD_MAIN_PLANE,             // main layer
				0,							// reserved
				0, 0, 0 					// layer masks ignored
		};

namespace rhfw {
namespace render {

class Win32OpenGl30RendererSurface final : public render::RenderTarget {
	friend class Win32OpenGl30RenderingContext;
private:
	Win32OpenGl30RenderingContext* context;
	core::Window* window;
	const HDC hdc;
	HWND hwnd;

	//-2 unset
	//-1 adaptive
	//0 - swapinterval
	int swapInterval = -2;
protected:
	virtual bool load() override {
		return true;
	}
	virtual void free() override {
		if (context->boundHdc == hdc) {
			context->bindHdc(NULL);
		}
	}

public:
	Win32OpenGl30RendererSurface(Win32OpenGl30RenderingContext* context, core::Window* window, HDC hdc)
			: context { context }, window { window }, hdc { hdc }, hwnd { window->getHwnd() } {
		load();
	}
	~Win32OpenGl30RendererSurface() {
		free();
	}

	void updateSwapInterval(int interval) {
		if (this->swapInterval == interval) {
			return;
		}
		if (context->wglfunc_wglSwapIntervalEXT != nullptr) {
			this->swapInterval = interval;
			context->bindHdc(hdc);
			BOOL res = context->wglfunc_wglSwapIntervalEXT(swapInterval);
			CHECK_SYSCALL_ERROR();
		}
	}

	virtual void bindForDrawing() override {
		context->bindHdc(hdc);
		if (context->renderer->getBoundFrameBufferName() != 0) {
			context->renderer->glBindFramebuffer(GL_FRAMEBUFFER, 0);
			CHECK_GL_ERROR_REND(context->renderer);
			context->renderer->setBoundFrameBufferName(0);
		}
	}
	virtual void finishDrawing() override {
		BOOL res;
		res = SwapBuffers(hdc);
		//res = wglSwapLayerBuffers(hdc, WGL_SWAP_MAIN_PLANE);
		CHECK_SYSCALL_ERROR() << " " << hdc;
	}
	virtual render::ViewPort getDefaultViewPort() override {
		return render::ViewPort { 0, 0, window->getWindowSize().pixelSize };
	}
};

#define LOAD_CONTEXT_PROC(name) wglfunc_##name = (PROTO_##name) wglfunc_wglGetProcAddress(STRINGIZE(name)); \
	WARN(wglfunc_##name == NULL) << "WGL method not found: " << STRINGIZE(name) << " " << GetLastError()

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

Win32OpenGl30RenderingContext::Win32OpenGl30RenderingContext(Renderer* renderer, LibraryHandle libhandle, const WglFunctions& wglfunc,
		HWND dummywindowhwnd, HDC dummywindowhdc, const RenderingContextOptions* options)
		: WglFunctions(wglfunc), renderer { static_cast<OpenGl30Renderer*>(renderer) }, libraryHandle(util::move(libhandle)), dummyWindowHwnd(
				dummywindowhwnd), dummyHdc(dummywindowhdc) {
	BOOL res;

	this->pixelFormat = getBasicPixelFormat();
	res = SetPixelFormat(dummyHdc, pixelFormat, &basicPixelFormatDescriptor);
	CHECK_SYSCALL_ERROR();

	HGLRC dummycontext = wglfunc_wglCreateContext(dummyHdc);
	ASSERT(dummycontext != NULL) << GetLastError();
	res = wglfunc_wglMakeCurrent(dummyHdc, dummycontext);
	CHECK_SYSCALL_ERROR();
	boundHdc = dummyHdc;

	initExtensionsList();

	if (options != nullptr) {
		contextOptions = *options;
	}

	if (contextOptions.multiSamplingFactor == 1 || (contextOptions.multiSamplingFactor > 1 && !hasMultisampleExtension)) {
		contextOptions.multiSamplingFactor = 0;
	}

	//if multisampling is enabled, get a new pixelformat
	if (wglfunc_wglChoosePixelFormatARB != nullptr && contextOptions.multiSamplingFactor > 1) {
		//get new pixelformat
		int newformat = reducePixelFormat(dummycontext);
		if (newformat >= 0) {
			this->pixelFormat = newformat;
		}
	}

	createAppropriateContext(dummycontext);
}
Win32OpenGl30RenderingContext::~Win32OpenGl30RenderingContext() {
	BOOL res;
	res = wglfunc_wglMakeCurrent(NULL, NULL);
	CHECK_SYSCALL_ERROR();
	res = wglfunc_wglDeleteContext(context);
	CHECK_SYSCALL_ERROR();
	res = ReleaseDC(dummyWindowHwnd, dummyHdc);
	CHECK_SYSCALL_ERROR();

	res = DestroyWindow(dummyWindowHwnd);
	CHECK_SYSCALL_ERROR();
}
void Win32OpenGl30RenderingContext::createAppropriateContext(HGLRC& dummycontext) {
	while (true) {
		if (createContext(dummycontext)) {
			break;
		} else {
			//failed to create context, try to reducing options
			if (wglfunc_wglChoosePixelFormatARB != nullptr && contextOptions.multiSamplingFactor > 1) {
				contextOptions.reduceMultiSamplingFactor();
				int newformat = reducePixelFormat(dummycontext);
				if (newformat >= 0) {
					this->pixelFormat = newformat;
				}
			} else {
				//cant reduce options anymore
				context = dummycontext;
				break;
			}
		}
	}
}
int Win32OpenGl30RenderingContext::reducePixelFormat(HGLRC& dummycontext) {
	int newformat = -1;
	do {
		newformat = getAppropriatePixelFormat();
		if (newformat >= 0) {
			BOOL res;
			res = wglfunc_wglMakeCurrent(NULL, NULL);
			CHECK_SYSCALL_ERROR();
			res = wglfunc_wglDeleteContext(dummycontext);
			CHECK_SYSCALL_ERROR();

			//An application can only set the pixel format of a window one time. Once a window's pixel format is set, it cannot be changed.
			//See Docs @SetPixelFormat
			//to work around this, we recreate the dummy window
			recreateDummyWindow();

			res = SetPixelFormat(dummyHdc, newformat, &basicPixelFormatDescriptor);
			CHECK_SYSCALL_ERROR();
			dummycontext = wglfunc_wglCreateContext(dummyHdc);
			ASSERT(dummycontext != NULL) << GetLastError();
			res = wglfunc_wglMakeCurrent(dummyHdc, dummycontext);
			CHECK_SYSCALL_ERROR();
			boundHdc = dummyHdc;

			initExtensionsList();
			break;
		}
		if (contextOptions.multiSamplingFactor == 0) {
			break;
		}
		contextOptions.reduceMultiSamplingFactor();
	} while (true);
	return newformat;
}

int Win32OpenGl30RenderingContext::getBasicPixelFormat() {
	BOOL res;

	int dummypixelformat = 0;
	dummypixelformat = ChoosePixelFormat(dummyHdc, &basicPixelFormatDescriptor);
	ASSERT(dummypixelformat != 0) << GetLastError();
	return dummypixelformat;
}
int Win32OpenGl30RenderingContext::getAppropriatePixelFormat() {
	int attributes[7 * 2 + 2 * 2 + 1] = { //
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, //
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE, //
					WGL_DOUBLE_BUFFER_ARB, GL_TRUE, //
					WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, //
					WGL_COLOR_BITS_ARB, 32, //
					WGL_DEPTH_BITS_ARB, 24, //
					WGL_STENCIL_BITS_ARB, 8, //
			};
	int attrslen = 7 * 2;
	if (contextOptions.multiSamplingFactor > 1) {
		attributes[attrslen++] = WGL_SAMPLE_BUFFERS_ARB;
		attributes[attrslen++] = 1;
		attributes[attrslen++] = WGL_SAMPLES_ARB;
		attributes[attrslen++] = contextOptions.multiSamplingFactor;
	}
	attributes[attrslen] = 0;

	int result = 0;
	UINT formatcountout = 0;
	BOOL res;
	res = wglfunc_wglChoosePixelFormatARB(dummyHdc, attributes, nullptr, 1, &result, &formatcountout);
	CHECK_SYSCALL_ERROR();
	if (!res || formatcountout == 0) {
		return -1;
	}
	return result;
}
bool Win32OpenGl30RenderingContext::reload() {
	//reload problably because of pixel format change, get a new one
	if (wglfunc_wglChoosePixelFormatARB != nullptr) {
		//get new pixelformat
		int newformat = reducePixelFormat(context);
		if (newformat >= 0) {
			this->pixelFormat = newformat;
		}
	}

	createAppropriateContext(context);
	return true;
}

void Win32OpenGl30RenderingContext::initExtensionsList() {
	LOAD_CONTEXT_PROC(wglGetExtensionsStringARB);
	ASSERT(wglfunc_wglGetExtensionsStringARB != nullptr);
	if (wglfunc_wglGetExtensionsStringARB != nullptr) {
		extensions = wglfunc_wglGetExtensionsStringARB(dummyHdc);
		ASSERT(extensions != nullptr);
		if (extensions == nullptr) {
			extensions = "";
		}
	} else {
		extensions = "";
	}
	LOGI()<< "WGL extensions: " << extensions;

	hasMultisampleExtension = hasExtensionString(extensions, "WGL_ARB_multisample");

	if (hasExtensionString(extensions, "WGL_ARB_pixel_format")) {
		LOAD_CONTEXT_PROC(wglChoosePixelFormatARB);
	} else {
		wglfunc_wglChoosePixelFormatARB = nullptr;
	}

	if (wglfunc_wglChoosePixelFormatARB == nullptr || !hasMultisampleExtension) {
		maximumMultisample = 0;
	}
}
bool Win32OpenGl30RenderingContext::createContext(HGLRC dummycontext) {
	bool result;
	BOOL res;

	if (hasExtensionString(extensions, "WGL_ARB_create_context")) {
		LOAD_CONTEXT_PROC(wglCreateContextAttribsARB);

		const int create_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		//WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#if RHFW_DEBUG && defined(_DEBUG)
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
				0 };

		context = wglfunc_wglCreateContextAttribsARB(dummyHdc, NULL, create_attribs);
		ASSERT(context != NULL) << GetLastError();
		res = wglfunc_wglMakeCurrent(dummyHdc, context);
		CHECK_SYSCALL_ERROR();
		boundHdc = dummyHdc;
		res = wglfunc_wglDeleteContext(dummycontext);
		CHECK_SYSCALL_ERROR();
		result = true;
	} else {
		wglfunc_wglCreateContextAttribsARB = nullptr;
		//no create context attribs
		result = false;
	}

	if (hasExtensionString(extensions, "WGL_EXT_swap_control")) {
		LOAD_CONTEXT_PROC(wglSwapIntervalEXT);
		LOAD_CONTEXT_PROC(wglGetSwapIntervalEXT);

		supportedVSyncOptions = VSyncOptions::VSYNC_ON | VSyncOptions::VSYNC_OFF;
		if (hasExtensionString(extensions, "WGL_EXT_swap_control_tear")) {
			supportedVSyncOptions |= VSyncOptions::ADAPTIVE;
		}
		if (contextOptions.vsyncOptions != VSyncOptions::NO_FLAG) {
			setVSyncOptions(contextOptions.vsyncOptions);
		}
	} else {
		supportedVSyncOptions = VSyncOptions::NO_FLAG;
		wglfunc_wglSwapIntervalEXT = nullptr;
		wglfunc_wglGetSwapIntervalEXT = nullptr;
	}
	LOGI()<< "Supported VSync options: " << supportedVSyncOptions;
	return result;
}

bool WglFunctions::init(HMODULE libhandle) {
#define LOAD_PROC(name) wglfunc_##name = (PROTO_##name) GetProcAddress(libhandle, STRINGIZE(name)); \
	if(wglfunc_##name == NULL) { LOGW() << "WGL method not found: " << STRINGIZE(name) << " " << GetLastError(); return false; }

	LOAD_PROC(wglGetProcAddress);
	LOAD_PROC(wglCreateContext);
	LOAD_PROC(wglMakeCurrent);
	LOAD_PROC(wglDeleteContext);

	return true;
}

void Win32OpenGl30RenderingContext::attachWindow(core::Window * window) {
	ASSERT(window != nullptr) << "Window to attach is nullptr";
	ASSERT(!window->getRenderSurface().isAttached()) << "Window is already attached";

	if ((window->getLastRenderer() != RenderConfig::OpenGl30 && window->getLastRenderer() != RenderConfig::_count_of_entries)
			|| (window->getPixelFormatIndex() >= 0 && window->getPixelFormatIndex() != pixelFormat)) {
		window->recreateWindow();
	} else {
		window->setLastRenderer(renderer->getRendererType());

		HWND hwnd = window->getHwnd();

		BOOL res;
		HDC hdc = GetDC(hwnd);
		if (window->getPixelFormatIndex() != pixelFormat) {
			res = SetPixelFormat(hdc, pixelFormat, &basicPixelFormatDescriptor);
			CHECK_SYSCALL_ERROR();
			window->setPixelFormatIndex(pixelFormat);
		}

		Resource<render::Win32OpenGl30RendererSurface> surface { new ResourceBlock( new Win32OpenGl30RendererSurface(this, window, hdc)) };
		window->getRenderSurface().set(renderer, surface);

		surface->updateSwapInterval(swapInterval);
	}
}
void Win32OpenGl30RenderingContext::detachWindow(core::Window * window) {
	if (window->getRenderSurface().getRenderTarget() == nullptr) {
		return;
	}
	if (window->getWindowStyle() == WindowStyle::FULLSCREEN) {
		HWND hwnd = static_cast<Win32OpenGl30RendererSurface&>(*window->getRenderSurface().getRenderTarget()).hwnd;
		//borderless fullscreen
		SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		//ShowWindow(hwnd, SW_RESTORE);
		//call SwapBuffers, to notify the driver about exiting full screen
		//some reference: https://www.reddit.com/r/vulkan/comments/4ep1pa/no_exclusive_fullscreen_ways_to_bypass_the_dwm/
		//not official reference, but it actually works on the development machine
		//SwapBuffers(((Win32OpenGl30RendererSurface*) window->getRenderSurface().getRenderTarget())->hdc);
		window->getRenderSurface().reset();
		SendMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		//ShowWindow(hwnd, SW_MAXIMIZE);
	} else {
		window->getRenderSurface().reset();
	}
}

void Win32OpenGl30RenderingContext::bindHdc(HDC hdc) {
	if (hdc == NULL) {
		hdc = dummyHdc;
	}
	if (boundHdc != hdc) {
		BOOL res;
		res = wglfunc_wglMakeCurrent(hdc, context);
		CHECK_SYSCALL_ERROR();
		boundHdc = hdc;
	}
}

void Win32OpenGl30RenderingContext::recreateDummyWindow() {
	if (boundHdc == dummyHdc) {
		boundHdc = NULL;
	}
	BOOL res;
	res = ReleaseDC(dummyWindowHwnd, dummyHdc);
	CHECK_SYSCALL_ERROR();
	res = DestroyWindow(dummyWindowHwnd);
	CHECK_SYSCALL_ERROR();

	dummyWindowHwnd = CreateWindowEx(0,	//extended style
			DUMMY_WINDOW_CLASS_NAME,	// name of the window class
			"",	// title of the window
			0,	// window style
			0, 0,	// x,y position of the window
			4, 4,	// width-height of the window
			HWND_MESSAGE,	// message window
			NULL,	// we aren't using menus, NULL
			win32platform::getApplicationInstance(),	// application handle
			NULL);	// no creation param
	ASSERT(dummyWindowHwnd != NULL) << GetLastError();

	dummyHdc = GetDC(dummyWindowHwnd);
	ASSERT(dummyHdc != NULL) << GetLastError();
}

VSyncOptions Win32OpenGl30RenderingContext::getSupportedVSyncOptions() {
	return supportedVSyncOptions;
}

RenderingContextOptionsResult Win32OpenGl30RenderingContext::setVSyncOptions(VSyncOptions options) {
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
			LOGW()<< "Unsupported VSync option: " << options;
			return RenderingContextOptionsResult::FAILED;
		}
	}
	//TODO change for attached windows
	for (auto& w : renderer->getAttachedWindows().objects()) {
		if (w.getRenderSurface().isAttached()) {
			auto& surface = static_cast<Win32OpenGl30RendererSurface&>(*w.getRenderSurface().getRenderTarget());
			surface.updateSwapInterval(swapInterval);
		}
	}
	this->contextOptions.vsyncOptions = options;
	return RenderingContextOptionsResult::SUCCESS;
}

RenderingContextOptionsResult Win32OpenGl30RenderingContext::setRenderingContextOptions(const RenderingContextOptions& options) {
	RenderingContextOptionsResult result = RenderingContextOptionsResult::NO_FLAG;
	unsigned int msfactor = options.multiSamplingFactor;
	if (msfactor == 1) {
		msfactor = 0;
	}
	if (msfactor != contextOptions.multiSamplingFactor) {
		//multisampling factor changed
		//only change if we have the appropriate extension
		if (hasMultisampleExtension) {
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

unsigned int Win32OpenGl30RenderingContext::getMaximumMultiSampleFactor() {
	if (maximumMultisample < 0) {
		int test = 2;
		while (true) {
			int attributes[] = { //
					WGL_SAMPLE_BUFFERS_ARB, 1, //
							WGL_SAMPLES_ARB, test, //
							0 };
			int result = 0;
			UINT formatcountout = 0;
			BOOL res;
			res = wglfunc_wglChoosePixelFormatARB(dummyHdc, attributes, nullptr, 1, &result, &formatcountout);
			CHECK_SYSCALL_ERROR();
			if (!res || formatcountout == 0) {
				break;
			}
			test *= 2;
		}
		maximumMultisample = (unsigned int) test / 2;
	}
	//TODO query this somehow
	return maximumMultisample;
}

} // namespace render

template<>
render::RenderingContext* instantiateRenderingContext<RenderConfig::OpenGl30>(render::Renderer* renderer,
		const render::RenderingContextOptions* options) {
	class DummyWindowRegisterer {
		DWORD error = ERROR_SUCCESS;
	public:
		DummyWindowRegisterer() {
			WNDCLASSEX wc;

			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = DefWindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = win32platform::getApplicationInstance();
			wc.hIcon = NULL;
			wc.hCursor = NULL;
			wc.hbrBackground = NULL;
			wc.lpszMenuName = NULL;
			wc.lpszClassName = DUMMY_WINDOW_CLASS_NAME;
			wc.hIconSm = NULL;

			auto regres = RegisterClassEx(&wc);
			if (regres == 0) {
				error = GetLastError();
				LOGW()<<"Failed to register class: " << error;
			}
		}

		operator bool() const {
			return error == ERROR_SUCCESS;
		}
	};
	static DummyWindowRegisterer onetime_registerer;
	if (onetime_registerer) {

		win32platform::WindowHandle dummywindowhandle { //
			CreateWindowEx(0,//extended style
					DUMMY_WINDOW_CLASS_NAME,// name of the window class
					"",// title of the window
					0,// window style
					CW_USEDEFAULT, CW_USEDEFAULT,// x,y position of the window
					4, 4,// width-height of the window
					HWND_MESSAGE,// message window
					NULL,// we aren't using menus, NULL
					win32platform::getApplicationInstance(),// application handle
					NULL)// no creation param
		};
		ASSERT(dummywindowhandle != NULL) << GetLastError();
		if (!dummywindowhandle) {
			return nullptr;
		}

		HDC dummyhdc = GetDC(dummywindowhandle);
		ASSERT(dummyhdc != NULL) << GetLastError();
		if (dummyhdc == NULL) {
			return nullptr;
		}

		LibraryHandle libhandle {"OpenGL32.dll"};
		if (!libhandle) {
			LOGW() << "Failed to load OpenGL library: " << GetLastError();
			BOOL res;
			res = ReleaseDC(dummywindowhandle, dummyhdc);
			CHECK_SYSCALL_ERROR();
			return nullptr;
		}
		render::WglFunctions wgl;
		if (!wgl.init((HMODULE) libhandle)) {
			LOGW() << "Failed to load WGL functions";
			BOOL res;
			res = ReleaseDC(dummywindowhandle, dummyhdc);
			CHECK_SYSCALL_ERROR();
			return nullptr;
		}
		return new render::Win32OpenGl30RenderingContext(renderer, util::move(libhandle), wgl, dummywindowhandle.take(), dummyhdc, options);
	}
	return nullptr;
}

}
	// namespace rhfw
