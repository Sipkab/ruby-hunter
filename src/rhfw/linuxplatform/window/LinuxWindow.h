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
 * LinuxWindow.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_LINUXWINDOW_H_
#define LINUXPLATFORM_LINUXWINDOW_H_

#include <framework/core/Window.h>

namespace rhfw {
namespace x11server {
class VisualInfoTracker;
}  // namespace x11server
namespace core {

class native_window_internal;

class LinuxWindow: public core::WindowBase {
private:
	friend class ::rhfw::platform_bridge;

	native_window_internal* nativeWindow = nullptr;

	WindowStyle windowStyle = WindowStyle::BORDERED;

	bool cursorVisible = true;

	LinuxWindow();
protected:
	virtual void showSoftKeyboardImpl(KeyboardType type) override;
	virtual void hideSoftKeyboardImpl() override;
public:
	~LinuxWindow();

	virtual void close() override;

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;

	virtual bool isHardwareKeyboardPresent() override {
		return true;
	}

	void recreateWindow(const x11server::VisualInfoTracker& visual);

	const native_window_internal* getNativeWindow() const {
		return nativeWindow;
	}

	virtual LinuxWindow* get() override {
		return this;
	}

	virtual WindowStyle getWindowStyle() override;
	virtual void setWindowStyle(WindowStyle style) override;
	virtual bool supportsWindowStyle(WindowStyle style) override;

	/**
	 * Call this only from the process main thread
	 */
	bool isCursorVisible() const;
	void setCursorVisible(bool visible);
};

} // namespace core
} // namespace rhfw

#endif /* LINUXPLATFORM_LINUXWINDOW_H_ */
