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
 * LinuxWindow.cpp
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#include <framework/render/Renderer.h>
#include <framework/core/timing.h>

#include <linuxplatform/window/LinuxWindow.h>
#include <linuxplatform/window/appglue.h>

#include <gen/log.h>

namespace rhfw {
namespace core {

LinuxWindow::LinuxWindow()
		: nativeWindow(new native_window_internal()) {
}
LinuxWindow::~LinuxWindow() {
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	if (nativeWindow->colorMap != 0) {
		xfunc_XFreeColormap(display, nativeWindow->colorMap);
	}
	nativeWindow->visualInfo.releaseLocked();

	if (nativeWindow->cursor != None) {
		xfunc_XFreeCursor(display, nativeWindow->cursor);
	}

	xfunc_XUnlockDisplay(display);
	delete nativeWindow;
}

void LinuxWindow::close() {
	LOGI()<< "Closing window";
	setVisibleToUser(false, core::MonotonicTime::getCurrent());

	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);

	xfunc_XDestroyWindow(display, nativeWindow->window);
	if (nativeWindow->colorMap != 0) {
		xfunc_XFreeColormap(display, nativeWindow->colorMap);
		nativeWindow->colorMap = 0;
	}
	nativeWindow->visualInfo.releaseLocked();
	//Have to XFlush here, because the main thread might not receive the event, because of the use of poll(...)

//	xfunc_XSync(display, False);
//	xfunc_XFlush(display);
	xfunc_XUnlockDisplay(display);
	x11server::notifyX11Loop();

	LOGI() << "Closed window";
}

void LinuxWindow::recreateWindow(const x11server::VisualInfoTracker& visual) {
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	xfunc_XDestroyWindow(display, nativeWindow->window);
	if (nativeWindow->colorMap != 0) {
		xfunc_XFreeColormap(display, nativeWindow->colorMap);
	}
	nativeWindow->visualInfo.releaseLocked();

	auto&& rootwindow = xfunc_XDefaultRootWindow(display);
	XSetWindowAttributes swa { 0 };
	swa.colormap = xfunc_XCreateColormap(display, rootwindow, visual->visual, AllocNone);
	swa.event_mask = X11_WINDOW_EVENT_MASKS;

	auto&& win = xfunc_XCreateWindow(display, rootwindow, 0, 0, 600, 600, 0, visual->depth, InputOutput, visual->visual,
			CWColormap | CWEventMask, &swa);

	Atom wmdelmsg = x11server::getWmDeleteMessageAtom();
	xfunc_XSetWMProtocols(display, win, &wmdelmsg, 1);
	//TODO fetch name instead
	xfunc_XStoreName(display, win, APPLICATION_DEFAULT_NAME);

	xfunc_XMapWindow(display, win);

	nativeWindow->window = win;
	nativeWindow->visualInfo = visual;
	nativeWindow->colorMap = swa.colormap;

//	xfunc_XSync(display, False);
//	xfunc_XFlush(display);
	xfunc_XUnlockDisplay(display);
	x11server::notifyX11Loop();
}

void LinuxWindow::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	attachedRenderer->attachWindow(this);
}
void LinuxWindow::detachFromRenderer() {
	attachedRenderer->detachWindow(this);
	WindowBase::detachFromRenderer();
}

void LinuxWindow::showSoftKeyboardImpl(KeyboardType type) {
}
void LinuxWindow::hideSoftKeyboardImpl() {
}

void LinuxWindow::setWindowStyle(WindowStyle style) {
	ASSERT(supportsWindowStyle(style)) << style;

	WindowStyle changes = style ^ windowStyle;
	auto oldstyle = this->windowStyle;
	this->windowStyle = style;

	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	if (HAS_FLAG(changes, WindowStyle::FULLSCREEN)) {
		XEvent xev { 0 };
		xev.type = ClientMessage;
		xev.xclient.window = nativeWindow->window;
		xev.xclient.message_type = x11server::getWmStateAtom();
		xev.xclient.format = 32;
		if (HAS_FLAG(style, WindowStyle::FULLSCREEN)) {
			xev.xclient.data.l[0] = _NET_WM_STATE_ADD;
		} else {
			xev.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
		}
		xev.xclient.data.l[1] = x11server::getWmStateFullscreenAtom();
		xev.xclient.data.l[2] = 0;

		xfunc_XSendEvent(display, xfunc_XDefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}
//	if (HAS_FLAG(changes, WindowStyle::BORDERED)) {
//		if (HAS_FLAG(oldstyle, WindowStyle::BORDERED)) {
//
//		} else {
//
//		}
//	}
	xfunc_XUnlockDisplay(display);
	x11server::notifyX11Loop();
}
bool LinuxWindow::supportsWindowStyle(WindowStyle style) {
	return style == WindowStyle::FULLSCREEN || style == WindowStyle::BORDERED; //|| (style == (WindowStyle::BORDERED | WindowStyle::FULLSCREEN));
}
WindowStyle LinuxWindow::getWindowStyle() {
	return windowStyle;
}

bool LinuxWindow::isCursorVisible() const {
	return cursorVisible;
}

void LinuxWindow::setCursorVisible(bool visible) {
	if (cursorVisible == visible) {
		return;
	}
	cursorVisible = visible;
	Display* display = x11server::getMainDisplay();
	xfunc_XLockDisplay(display);
	if (!visible) {
		//hide the cursor, create blank one
		if (nativeWindow->cursor == None) {
			static char data[4] = { 0 };
			XColor dummy;

			Pixmap blank = xfunc_XCreateBitmapFromData(display, xfunc_XDefaultRootWindow(display), data, 1, 1);
			XDEBUGSYNC(display);
			if (blank != None) {
				Cursor cursor = xfunc_XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);
				XDEBUGSYNC(display);
				xfunc_XFreePixmap(display, blank);
				nativeWindow->cursor = cursor;
				XDEBUGSYNC(display);
			}
		}
		xfunc_XDefineCursor(display, nativeWindow->window, nativeWindow->cursor);
	} else {
		xfunc_XDefineCursor(display, nativeWindow->window, None);
	}
	XDEBUGSYNC(display);
	xfunc_XUnlockDisplay(display);
}

} // namespace core
} // namespace rhfw
