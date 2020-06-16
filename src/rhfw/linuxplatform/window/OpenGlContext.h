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
 * OpenGlContext.h
 *
 *  Created on: 2017. febr. 11.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_OPENGLCONTEXT_H_
#define LINUXPLATFORM_OPENGLCONTEXT_H_

#include <opengl30/OpenGl30Renderer.h>

namespace rhfw {
namespace render {
class LinuxWindowRenderSurface;

class OpenGlContext final: public render::RenderingContext {
private:
	friend LinuxWindowRenderSurface;

	OpenGl30Renderer* renderer;
	GLXContext context;
	core::Window* currentWindow = nullptr;
	Pixmap dummyPixmap;
	GLXPixmap dummyGlxPixmap;

	int swapInterval = 0;

	const char* glxextensions = "";

	x11server::VisualInfoTracker visualInfo;

	VSyncOptions supportedVSyncOptions = VSyncOptions::NO_FLAG;

	typedef void (*PROTO_glXSwapIntervalEXT)(Display *dpy, GLXDrawable drawable, int interval);
	PROTO_glXSwapIntervalEXT glxfunc_glXSwapIntervalEXT = nullptr;

	bool hasExtensionGLX_EXT_swap_control_tear = false;
	bool hasMultisampleExtensions = false;

	int maximumMultisample = -1;

	x11server::VisualInfoTracker getAppropriateVisual(Display* display, const RenderingContextOptions& options, GLXFBConfig* outfbconfig);

	bool createAppropriateContext(Display* display);
public:
	OpenGlContext(OpenGl30Renderer* renderer, const render::RenderingContextOptions* options);

	~OpenGlContext();

	virtual void attachWindow(core::Window* window) override;
	virtual void detachWindow(core::Window* window) override;

	void setWindow(core::Window* window);
	void setWindowLocked(Display* display, core::Window* window);

	virtual unsigned int getMaximumMultiSampleFactor() override;
	virtual VSyncOptions getSupportedVSyncOptions() override;
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) override;

	virtual RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) override;

	virtual bool reload() override;
};

}  // namespace render
}  // namespace rhfw

#endif /* LINUXPLATFORM_OPENGLCONTEXT_H_ */
