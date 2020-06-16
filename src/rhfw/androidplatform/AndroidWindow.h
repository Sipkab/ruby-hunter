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
 * AndroidWindow.h
 *
 *  Created on: 2016. febr. 27.
 *      Author: sipka
 */

#ifndef ANDROIDWINDOW_H_
#define ANDROIDWINDOW_H_

#include <framework/core/Window.h>
#include <framework/io/key/UnicodeCodePoint.h>

#include <gen/configuration.h>

struct ANativeWindow;
struct ANativeActivity;
struct AInputQueue;

namespace rhfw {
namespace core {

class AndroidWindow final: public WindowBase {
private:
	friend class ::rhfw::platform_bridge;

	ANativeActivity* activity = nullptr;
	ANativeWindow* window = nullptr;
	AInputQueue* inputQueue = nullptr;

	UnicodeCodePoint previousAccent;

	bool finishing = false;

	bool shouldBeVisibleToUser = false;

	float xdpi;
	float ydpi;

	bool isLandscape = false;

	void setNativeWindow(ANativeWindow* window, core::time_micros time);

	void internalSetSize(unsigned int width, unsigned int height) {
		if (isLandscape) {
			setSize(core::WindowSize { Size2UI { width, height }, Vector2F { ydpi, xdpi } });
		} else {
			setSize(core::WindowSize { Size2UI { width, height }, Vector2F { xdpi, ydpi } });
		}
	}

	void setContentRect(unsigned int width, unsigned int height) {
		internalSetSize(width, height);
	}

	bool hasNativeWindow() const {
		return window != nullptr;
	}

	AndroidWindow(ANativeActivity* activity, float xdpi, float ydpi)
			: activity { activity }, xdpi { xdpi }, ydpi { ydpi } {
	}
protected:
	virtual void setVisibleToUser(bool visible, core::time_micros time) override;

	virtual void showSoftKeyboardImpl(KeyboardType type) override;
	virtual void hideSoftKeyboardImpl() override;
public:
	~AndroidWindow();

	virtual AndroidWindow* get() override {
		return this;
	}

	ANativeWindow* getNativeWindow() {
		return window;
	}

	ANativeActivity* getNativeActivity() {
		return activity;
	}

	virtual void close() override;

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;

	virtual bool isHardwareKeyboardPresent() override;

	virtual WindowStyle getWindowStyle() override {
		return WindowStyle::FULLSCREEN;
	}
	virtual void setWindowStyle(WindowStyle style) override {
		ASSERT(supportsWindowStyle(style));
		//ignore, only one style is accepted
	}
	virtual bool supportsWindowStyle(WindowStyle style) override {
		return style == WindowStyle::FULLSCREEN;
	}
};

} // namespace core
} // namespace rhfw

#endif /* ANDROIDWINDOW_H_ */
