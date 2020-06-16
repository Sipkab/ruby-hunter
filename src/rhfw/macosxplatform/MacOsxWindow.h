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
//
//  MacOsxWindow.h
//  TestApp
//
//  Created by User on 2016. 08. 30..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef MacOsxWindow_h
#define MacOsxWindow_h

#include <framework/core/Window.h>

#include <gen/configuration.h>

class macosx_native_window_cpp;

namespace rhfw {
namespace core {

class MacOsxWindow: public WindowBase {
private:
	friend class ::rhfw::platform_bridge;

	macosx_native_window_cpp* nativeWindow = nullptr;

	WindowStyle windowStyle = WindowStyle::NO_FLAG;

	bool cursorVisible = true;

	void setNativeWindow(macosx_native_window_cpp* window, core::time_micros time);

	MacOsxWindow() {
	}
protected:
	virtual void showSoftKeyboardImpl(KeyboardType type) override;
	virtual void hideSoftKeyboardImpl() override;
public:
	~MacOsxWindow();

	MacOsxWindow* get() override {
		return this;
	}

	virtual void close() override;

	virtual bool isHardwareKeyboardPresent() override {
		return true;
	}

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;

	macosx_native_window_cpp* getNativeWindow() const {
		return nativeWindow;
	}

	bool hasNativeWindow() const {
		return getNativeWindow() != nullptr;
	}

	bool isCursorVisible() const {
		return cursorVisible;
	}
	void setCursorVisible(bool visible);

	virtual WindowStyle getWindowStyle() override;
	virtual void setWindowStyle(WindowStyle style) override;
	virtual bool supportsWindowStyle(WindowStyle style) override;
};

} // namespace core
} // namespace rhfw

#endif /* MacOsxWindow_h */
