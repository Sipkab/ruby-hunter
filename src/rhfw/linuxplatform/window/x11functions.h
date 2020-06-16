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
 * x11functions.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_X11FUNCTIONS_H_
#define LINUXPLATFORM_X11FUNCTIONS_H_

#include <X11/X.h>
#include <GL/glx.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/platform.h>

typedef Display* (*PROTO_XOpenDisplay)(char* display_name);
typedef Window (*PROTO_XDefaultRootWindow)(Display* display);
typedef Colormap (*PROTO_XCreateColormap)(Display* display, Window w, Visual* visual, int alloc);
typedef Window (*PROTO_XCreateWindow)(Display* display, Window parent, int x, int y, unsigned int width, unsigned int height,
		unsigned int border_width, int depth, unsigned int class_, Visual* visual, unsigned long valuemask,
		XSetWindowAttributes* attributes);
typedef int (*PROTO_XNextEvent)(Display* display, XEvent* event_return);
typedef int (*PROTO_XEventsQueued)(Display* display, int mode);
typedef Status (*PROTO_XSendEvent)(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send);
typedef int (*PROTO_XPending)(Display* display);
typedef int (*PROTO_XDestroyWindow)(Display* display, Window w);
typedef int (*PROTO_XCloseDisplay)(Display* display);
typedef int (*PROTO_XMapWindow)(Display* display, Window w);
typedef int (*PROTO_XStoreName)(Display* display, Window w, const char* windowname);
typedef Pixmap (*PROTO_XCreatePixmap)(Display* display, Drawable d, unsigned int width, unsigned int height, unsigned int depth);
typedef int (*PROTO_XFreePixmap)(Display* display, Pixmap pixmap);
typedef Status (*PROTO_XInitThreads)();
typedef Status (*PROTO_XLockDisplay)(Display* display);
typedef Status (*PROTO_XUnlockDisplay)(Display* display);
typedef KeySym (*PROTO_XKeycodeToKeysym)(Display* display, KeyCode keycode, int index);
typedef int (*PROTO_XLookupString)(XKeyEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return,
		XComposeStatus *status_return);
typedef Bool (*PROTO_XFilterEvent)(XEvent* event, Window win);
typedef int (*PROTO_XFree)(void* data);
typedef Atom (*PROTO_XInternAtom)(Display* display, const char* atomname, Bool only_if_exists);
typedef Status (*PROTO_XSetWMProtocols)(Display* display, Window w, Atom* protocols, int count);
typedef int (*PROTO_XDisplayWidthMM)(Display* display, int screen_number);
typedef int (*PROTO_XDisplayHeightMM)(Display* display, int screen_number);
typedef int (*PROTO_XDisplayWidth)(Display* display, int screen_number);
typedef int (*PROTO_XDisplayHeight)(Display* display, int screen_number);
typedef int (*PROTO_XDefaultScreen)(Display* display);
typedef int (*PROTO_XConnectionNumber)(Display* display);
typedef int (*PROTO_XFreeColormap)(Display *display, Colormap colormap);
typedef int (*PROTO_XFlush)(Display *display);
typedef int (*PROTO_XSync)(Display *display, Bool discard);
typedef int (*PROTO_XSetErrorHandler)(int (*handler)(Display *, XErrorEvent *));

typedef int (*PROTO_XDefineCursor)(Display *, Window, Cursor);
typedef Pixmap (*PROTO_XCreateBitmapFromData)(Display *display, Drawable d, char *data, unsigned int width, unsigned int height);
typedef int (*PROTO_XFreeCursor)(Display *display, Cursor cursor);
typedef Cursor (*PROTO_XCreatePixmapCursor)(Display *display, Pixmap source, Pixmap mask, XColor *foreground_color, XColor *background_color, unsigned int x, unsigned int y);

#define DECLARE_XFUNCTION(name) extern PROTO_##name xfunc_##name

DECLARE_XFUNCTION(XOpenDisplay);
DECLARE_XFUNCTION(XDefaultRootWindow);
DECLARE_XFUNCTION(XCreateColormap);
DECLARE_XFUNCTION(XCreateWindow);
DECLARE_XFUNCTION(XNextEvent);
DECLARE_XFUNCTION(XEventsQueued);
DECLARE_XFUNCTION(XSendEvent);
DECLARE_XFUNCTION(XPending);
DECLARE_XFUNCTION(XDestroyWindow);
DECLARE_XFUNCTION(XCloseDisplay);
DECLARE_XFUNCTION(XMapWindow);
DECLARE_XFUNCTION(XStoreName);
DECLARE_XFUNCTION(XCreatePixmap);
DECLARE_XFUNCTION(XFreePixmap);
DECLARE_XFUNCTION(XInitThreads);
DECLARE_XFUNCTION(XLockDisplay);
DECLARE_XFUNCTION(XUnlockDisplay);
DECLARE_XFUNCTION(XKeycodeToKeysym);
DECLARE_XFUNCTION(XLookupString);
DECLARE_XFUNCTION(XFilterEvent);
DECLARE_XFUNCTION(XFree);
DECLARE_XFUNCTION(XInternAtom);
DECLARE_XFUNCTION(XSetWMProtocols);
DECLARE_XFUNCTION(XDisplayWidthMM);
DECLARE_XFUNCTION(XDisplayHeightMM);
DECLARE_XFUNCTION(XDisplayWidth);
DECLARE_XFUNCTION(XDisplayHeight);
DECLARE_XFUNCTION(XDefaultScreen);
DECLARE_XFUNCTION(XConnectionNumber);
DECLARE_XFUNCTION(XFreeColormap);
DECLARE_XFUNCTION(XFlush);
DECLARE_XFUNCTION(XSync);
DECLARE_XFUNCTION(XSetErrorHandler);
DECLARE_XFUNCTION(XDefineCursor);
DECLARE_XFUNCTION(XCreateBitmapFromData);
DECLARE_XFUNCTION(XFreeCursor);
DECLARE_XFUNCTION(XCreatePixmapCursor);

#undef DECLARE_XFUNCTION

#if RHFW_DEBUG
#define XDEBUGSYNC(display) xfunc_XSync(display, false)
#else
#define XDEBUGSYNC(display)
#endif /* RHFW_DEBUG */

namespace rhfw {
namespace x11server {
Display* getMainDisplay();
Atom getWmDeleteMessageAtom();
Atom getWmStateFullscreenAtom();
Atom getWmStateAtom();
void notifyX11Loop();

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

class VisualInfoTracker {
	XVisualInfo* info = nullptr;
	unsigned int* refCount = nullptr;

	void decrease() {
		if (refCount != nullptr && --(*refCount) == 0) {
			delete refCount;

			Display* display = getMainDisplay();
			xfunc_XLockDisplay(display);
			xfunc_XFree(info);
			xfunc_XUnlockDisplay(display);
		}
		info = nullptr;
		refCount = nullptr;
	}
public:
	VisualInfoTracker() {
	}
	VisualInfoTracker(NULLPTR_TYPE) {
	}
	~VisualInfoTracker() {
		decrease();
	}
	static VisualInfoTracker make(XVisualInfo* info) {
		VisualInfoTracker result;
		result.info = info;
		result.refCount = new unsigned int(1);
		//move
		return static_cast<VisualInfoTracker&&>(result);
	}
	VisualInfoTracker(const VisualInfoTracker& o)
			: info(o.info), refCount(o.refCount) {
		++(*refCount);
	}
	VisualInfoTracker(VisualInfoTracker&& o)
			: info(o.info), refCount(o.refCount) {
		o.info = nullptr;
		o.refCount = nullptr;
	}
	VisualInfoTracker& operator=(const VisualInfoTracker& o) {
		decrease();

		this->info = o.info;
		this->refCount = o.refCount;
		++(*refCount);
		return *this;
	}
	VisualInfoTracker& operator=(VisualInfoTracker&& o) {
		decrease();

		this->info = o.info;
		this->refCount = o.refCount;

		o.info = nullptr;
		o.refCount = nullptr;
		return *this;
	}

	VisualInfoTracker& operator=(NULLPTR_TYPE) {
		decrease();
		return *this;
	}
	bool operator==(NULLPTR_TYPE) const {
		return info == nullptr;
	}
	bool operator!=(NULLPTR_TYPE) const {
		return info != nullptr;
	}

	operator XVisualInfo*() const {
		return info;
	}

	XVisualInfo* operator->() const {
		return info;
	}

	void releaseLocked() {
		if (refCount != nullptr && --(*refCount) == 0) {
			delete refCount;

			xfunc_XFree(info);
		}
		info = nullptr;
		refCount = nullptr;
	}
};

}  // namespace x11server
}  // namespace rhfw

#endif /* LINUXPLATFORM_X11FUNCTIONS_H_ */
