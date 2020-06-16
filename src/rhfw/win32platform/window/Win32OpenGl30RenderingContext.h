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
#ifndef OPENGL30RENDERINGCONTEXT_H_
#define OPENGL30RENDERINGCONTEXT_H_

#include <framework/render/RenderingContext.h>
#include <win32platform/LibraryHandle.h>

namespace rhfw {
class OpenGl30Renderer;
namespace render {

class WglFunctions {
public:
	typedef HGLRC (WINAPI * PROTO_wglCreateContextAttribsARB) (HDC hDC, HGLRC hShareContext, const int *attribList);

	typedef PROC (WINAPI * PROTO_wglGetProcAddress)(LPCSTR);

	typedef HGLRC (WINAPI * PROTO_wglCreateContext)(HDC);

	typedef BOOL (WINAPI* PROTO_wglMakeCurrent)(HDC, HGLRC);

	typedef BOOL (WINAPI* PROTO_wglDeleteContext)(HGLRC);

	typedef BOOL (WINAPI * PROTO_wglSwapIntervalEXT)(int interval);

	typedef int (WINAPI * PROTO_wglGetSwapIntervalEXT)();

	typedef const char* (WINAPI * PROTO_wglGetExtensionsStringARB)(HDC hdc);

	typedef BOOL (WINAPI * PROTO_wglChoosePixelFormatARB)(HDC hdc,
			const int *piAttribIList,
			const FLOAT *pfAttribFList,
			UINT nMaxFormats,
			int *piFormats,
			UINT *nNumFormats);

	//DLL exported functions
	PROTO_wglGetProcAddress wglfunc_wglGetProcAddress = nullptr;
	PROTO_wglCreateContext wglfunc_wglCreateContext = nullptr;
	PROTO_wglMakeCurrent wglfunc_wglMakeCurrent = nullptr;
	PROTO_wglDeleteContext wglfunc_wglDeleteContext = nullptr;

	//WGL extension functions
	PROTO_wglCreateContextAttribsARB wglfunc_wglCreateContextAttribsARB = nullptr;

	PROTO_wglSwapIntervalEXT wglfunc_wglSwapIntervalEXT = nullptr;
	PROTO_wglGetSwapIntervalEXT wglfunc_wglGetSwapIntervalEXT = nullptr;

	PROTO_wglGetExtensionsStringARB wglfunc_wglGetExtensionsStringARB = nullptr;

	PROTO_wglChoosePixelFormatARB wglfunc_wglChoosePixelFormatARB = nullptr;

	bool init(HMODULE libhandle);
};

class Win32OpenGl30RendererSurface;
class Win32OpenGl30RenderingContext: public render::RenderingContext, public WglFunctions {
	friend class Win32OpenGl30RendererSurface;
private:
	OpenGl30Renderer* renderer = nullptr;
	LibraryHandle libraryHandle;

	HWND dummyWindowHwnd = NULL;

	HDC dummyHdc = NULL;
	HGLRC context = NULL;
	int pixelFormat = -1;
	HDC boundHdc = NULL;

	const char* extensions = "";

	VSyncOptions supportedVSyncOptions = VSyncOptions::NO_FLAG;

	int swapInterval = 0;

	int maximumMultisample = -1;
	bool hasMultisampleExtension = false;

	bool createContext(HGLRC dummycontext);

	void initExtensionsList();

	int getBasicPixelFormat();
	int getAppropriatePixelFormat();
	int reducePixelFormat(HGLRC& dummycontext);

	void recreateDummyWindow();

	void createAppropriateContext(HGLRC& dummycontext);
protected:
	virtual bool reload() override;
public:

	Win32OpenGl30RenderingContext(Renderer* renderer, LibraryHandle libhandle, const WglFunctions& wglfunc, HWND dummywindowhwnd,
			HDC dummywindowhdc, const RenderingContextOptions* options);
	~Win32OpenGl30RenderingContext();

	virtual void attachWindow(core::Window* window) override;
	virtual void detachWindow(core::Window* window) override;

	void bindHdc(HDC hdc);

	OpenGl30Renderer* getRenderer() {
		return renderer;
	}

	HMODULE getLibrary() const {
		return (HMODULE) libraryHandle;
	}

	virtual VSyncOptions getSupportedVSyncOptions() override;
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) override;

	virtual unsigned int getMaximumMultiSampleFactor() override;

	virtual RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) override;
};
} // namespace render
} // namespace rhfw

#endif /* OPENGL30RENDERINGCONTEXT_H_ */
