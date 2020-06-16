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
#ifndef WIN32PLATFORM_WIN32WINDOW_H_
#define WIN32PLATFORM_WIN32WINDOW_H_

#include <framework/core/Window.h>

#include <gen/configuration.h>

#include <win32platform/minwindows.h>

namespace rhfw {
namespace core {

#define WIN32WINDOW_CLASSNAME "Win32WindowClass"

class Win32Window final : public WindowBase {
private:
	//TODO disable window style change when in exclusive fullscreen mode, and apply it later.
	friend class ::rhfw::platform_bridge;

	WindowStyle appliedWindowStyle = WindowStyle::NO_FLAG;
	WindowStyle windowStyle = WindowStyle::NO_FLAG;

	RenderConfig lastRenderer = RenderConfig::_count_of_entries;

	unsigned int lastHighSurrogate = 0;

	HWND hwnd = NULL;

	bool shouldBeVisibleToUser = false;

	int currentPixelFormat = -1;

	bool cursorVisible = true;
	bool clipCursor = false;

	Win32Window(WindowStyle style);

	HWND updateHwnd(HWND hwnd);
	void finishHwndUpdate(HWND oldhwnd, core::time_micros time);
	void setHwnd(HWND hwnd, core::time_micros time);

protected:
	virtual void setVisibleToUser(bool visible, core::time_micros time) override;

	//ignore soft keyboard on win32
	virtual void showSoftKeyboardImpl(KeyboardType type) override {
	}
	virtual void hideSoftKeyboardImpl() override {
	}
public:
	~Win32Window();
	Win32Window* get() override {
		return this;
	}

	int getPixelFormatIndex() const {
		return currentPixelFormat;
	}

	void setPixelFormatIndex(int index) {
		ASSERT(currentPixelFormat == -1) << "Can't set pixelformat multiple times. Recreate window first.";
		this->currentPixelFormat = index;
	}

	virtual void close() override;

	virtual bool isReadyToDraw() override {
		return WindowBase::isReadyToDraw() && size.pixelSize.width() > 0 && size.pixelSize.height() > 0;
	}

	BOOL show(int showcmd);

	HWND getHwnd() {
		return hwnd;
	}

	const char* getWindowTitle() {
		return APPLICATION_DEFAULT_NAME;
	}

	void recreateWindow();

	bool hasHwnd() const {
		return hwnd != NULL;
	}

	void setLastRenderer(RenderConfig renderer) {
		this->lastRenderer = renderer;
	}
	RenderConfig getLastRenderer() {
		return lastRenderer;
	}

	virtual bool isHardwareKeyboardPresent() override {
		return true;
	}

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;

	virtual WindowStyle getWindowStyle() override;
	virtual void setWindowStyle(WindowStyle style) override;
	virtual bool supportsWindowStyle(WindowStyle style) override;

	/**
	 * Call this only from the process main thread
	 */
	bool isCursorVisible() const {
		return cursorVisible;
	}
	void setCursorVisible(bool visible);

	/**
	 * Call this only from the process main thread
	 */
	bool isClipCursor() const {
		return clipCursor;
	}
	void setClipCursor(bool clip);
};

} // namespace core
} // namespace rhfw

//define will collide with DrawState class in Renderer
#undef DrawState

#endif /* WIN32PLATFORM_WIN32WINDOW_H_ */
