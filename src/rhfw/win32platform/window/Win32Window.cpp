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
#include <windows.h>

#include <win32platform/window/Win32Platform.h>
#include <win32platform/window/Win32Window.h>
#include <framework/render/Renderer.h>

#include <win32platform/window/MessageDefinitions.h>

namespace rhfw {
namespace core {

Win32Window::Win32Window(WindowStyle style)
		: appliedWindowStyle(style), windowStyle(style) {
}
Win32Window::~Win32Window() {
}

void Win32Window::setVisibleToUser(bool visible, core::time_micros time) {
	shouldBeVisibleToUser = visible;
	WindowBase::setVisibleToUser(visible && hasHwnd(), time);
}

void Win32Window::setHwnd(HWND hwnd, core::time_micros time) {
	if (this->hwnd != NULL) {
		if (isAttachedToRenderer()) {
			attachedRenderer->detachWindow(this);
		}
	}
	this->hwnd = hwnd;
	currentPixelFormat = -1;
	if (this->hwnd != NULL) {
		if (isAttachedToRenderer()) {
			attachedRenderer->attachWindow(this);
		}
	}

	WindowBase::setVisibleToUser(shouldBeVisibleToUser && hasHwnd(), time);
}

HWND Win32Window::updateHwnd(HWND hwnd) {
	HWND old = this->hwnd;
	this->hwnd = hwnd;
	return old;
}
void Win32Window::finishHwndUpdate(HWND oldhwnd, core::time_micros time) {
	if (oldhwnd != NULL) {
		if (isAttachedToRenderer()) {
			attachedRenderer->detachWindow(this);
		}
	}
	currentPixelFormat = -1;
	if (this->hwnd != NULL) {
		if (isAttachedToRenderer()) {
			attachedRenderer->attachWindow(this);
		}
	}

	WindowBase::setVisibleToUser(shouldBeVisibleToUser && hasHwnd(), time);
}

void Win32Window::close() {
	BOOL res = PostMessage(hwnd, WM_USER_DESTROY_WINDOW, 0, 0);
	ASSERT(res) << GetLastError();
}

BOOL Win32Window::show(int showcmd) {
	return ShowWindowAsync(hwnd, showcmd);
}

void Win32Window::recreateWindow() {
	ASSERT(this->hwnd != NULL);
	BOOL res = PostMessage(hwnd, WM_USER_RECREATE_WINDOW, NULL, NULL);
	ASSERT(res) << GetLastError();

	//setHwnd(NULL, core::MonotonicTime::getCurrent());
	lastRenderer = RenderConfig::_count_of_entries;
}

void Win32Window::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	if (hasHwnd()) {
		attachedRenderer->attachWindow(this);
	}
}

void Win32Window::detachFromRenderer() {
	if (isAttachedToRenderer() && hasHwnd()) {
		attachedRenderer->detachWindow(this);
	}
	WindowBase::detachFromRenderer();
}

WindowStyle Win32Window::getWindowStyle() {
	return windowStyle;
}

void Win32Window::setWindowStyle(WindowStyle style) {
	ASSERT(supportsWindowStyle(style)) << style;
	WindowStyle changes = style ^ windowStyle;
	if (changes == WindowStyle::NO_FLAG) {
		return;
	}
	LOGTRACE()<< "Queue style change: " << hwnd << " to: " <<style;
	auto oldstyle = this->windowStyle;
	this->windowStyle = style;

	PostThreadMessage(win32platform::getMainThreadId(), WM_USER_EXECUTE, NULL, (LPARAM) WinMessageRunnable::from([=] () mutable {
		appliedWindowStyle = windowStyle;
		LOGTRACE() << "Set window style: " << hwnd << " to: " << style;
		if (HAS_FLAG(changes, WindowStyle::BORDERED)) {
			LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
			if (HAS_FLAG(style, WindowStyle::BORDERED)) {
				lStyle = (lStyle | (WS_CAPTION | WS_THICKFRAME | WS_SYSMENU)) & ~WS_POPUP;
			} else {
				lStyle = (lStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU)) | WS_POPUP;
			}
			SetWindowLongPtr(hwnd, GWL_STYLE, lStyle);
			if (HAS_FLAG(oldstyle, WindowStyle::FULLSCREEN)) {
				/*In case of fullscreen, we need to remaximize the window,
				 or else the window might get bigger than the actual screen
				 this updates the style too*/
				ShowWindow(hwnd, SW_RESTORE);
				changes |= WindowStyle::FULLSCREEN;
			} else {
				/*as in the documentation, we need to call SetWindowPos to update chached value*/
				SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}

		if (HAS_FLAG(changes, WindowStyle::FULLSCREEN)) {
			if (HAS_FLAG(style, WindowStyle::FULLSCREEN)) {
				if (HAS_FLAG(style, WindowStyle::BORDERED)) {
					ShowWindow(hwnd, SW_MAXIMIZE);
				} else {
					/*TODO go native fullscreen, via renderingcontext
					 or should we create a new function to enter exclusive?*/
					ShowWindow(hwnd, SW_MAXIMIZE);
				}
			} else {
				ShowWindow(hwnd, SW_RESTORE);
			}
		}
	}));
}

bool Win32Window::supportsWindowStyle(WindowStyle style) {
	return true;
}

void Win32Window::setCursorVisible(bool visible) {
	BOOL res = PostMessage(hwnd, WM_USER_CURSOR_VISIBILITY, visible ? 1 : 0, 0);
	ASSERT(res) << GetLastError();
}
void Win32Window::setClipCursor(bool clip) {
	BOOL res = PostMessage(hwnd, WM_USER_CURSOR_CLIPPING, clip ? 1 : 0, 0);
	ASSERT(res) << GetLastError();
}

} // namespace core
} // namespace rhfw

