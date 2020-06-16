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
 * glxfunctions.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_GLXFUNCTIONS_H_
#define LINUXPLATFORM_GLXFUNCTIONS_H_

#include <X11/X.h>
#include <GL/glx.h>

typedef void* (*PROTO_glXGetProcAddress)(const GLubyte* procname);
typedef XVisualInfo* (*PROTO_glXChooseVisual)(Display* dpy, int screen, int* attriblist);
typedef Bool (*PROTO_glXMakeCurrent)(Display* dpy, GLXDrawable drawable, GLXContext context);
typedef GLXContext (*PROTO_glXCreateContext)(Display* dpy, XVisualInfo* vis, GLXContext context, Bool direct);
typedef void (*PROTO_glXDestroyContext)(Display* dpy, GLXContext context);
typedef void (*PROTO_glXSwapBuffers)(Display* dpy, GLXDrawable drawable);
typedef const char * (*PROTO_glXQueryExtensionsString)(Display * dpy, int screen);
typedef GLXFBConfig* (*PROTO_glXChooseFBConfig)(Display * dpy, int screen, const int * attrib_list, int * nelements);
typedef XVisualInfo* (*PROTO_glXGetVisualFromFBConfig)(Display * dpy, GLXFBConfig config);
typedef GLXPixmap (*PROTO_glXCreateGLXPixmap)(Display * dpy, XVisualInfo * vis, Pixmap pixmap);
typedef void (*PROTO_glXDestroyGLXPixmap)(Display * dpy, GLXPixmap pix);

#define DECLARE_GLXFUNCTION(name) extern PROTO_##name glxfunc_##name

DECLARE_GLXFUNCTION(glXGetProcAddress);
DECLARE_GLXFUNCTION(glXChooseVisual);
DECLARE_GLXFUNCTION(glXMakeCurrent);
DECLARE_GLXFUNCTION(glXCreateContext);
DECLARE_GLXFUNCTION(glXDestroyContext);
DECLARE_GLXFUNCTION(glXSwapBuffers);
DECLARE_GLXFUNCTION(glXQueryExtensionsString);
DECLARE_GLXFUNCTION(glXChooseFBConfig);
DECLARE_GLXFUNCTION(glXGetVisualFromFBConfig);
DECLARE_GLXFUNCTION(glXCreateGLXPixmap);
DECLARE_GLXFUNCTION(glXDestroyGLXPixmap);

#undef DECLARE_GLXFUNCTION

#endif /* LINUXPLATFORM_GLXFUNCTIONS_H_ */
