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
//  IosWindow.h
//  TestApp
//
//  Created by User on 2016. 03. 01..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef IosWindow_h
#define IosWindow_h

#include <framework/core/Window.h>

#include <gen/configuration.h>

struct ios_native_layer;

namespace rhfw {
namespace core {

class IosWindow: public WindowBase {
private:
	friend class ::rhfw::platform_bridge;

	struct ios_native_layer* nativeLayer = nullptr;

	void setNativeLayer(struct ios_native_layer* nativeLayer, core::time_micros time);
	bool hasNativeLayer() const {
		return nativeLayer != nullptr;
	}

	IosWindow() {
	}
protected:
	virtual void showSoftKeyboardImpl(KeyboardType type) override;
	virtual void hideSoftKeyboardImpl() override;
public:
	~IosWindow() {
	}

	IosWindow* get() override {
		return this;
	}

	virtual void close() override {
	}

	virtual bool isHardwareKeyboardPresent() override {
		return false;
	}

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;

	struct ios_native_layer* getNativeLayer() {
		return nativeLayer;
	}

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

#endif /* IosWindow_h */
