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
#ifndef WINSTORE_WINDOW_H_
#define WINSTORE_WINDOW_H_

#include <framework/core/Window.h>

#include <gen/configuration.h>
#include <Windows.h>

namespace rhfw {
namespace core {

#pragma warning(push)
#pragma warning(disable: 4451) // marshalling warning, but nativeWindow will be only used from main thread
class WinstoreWindow final : public WindowBase {
private:
	friend class ::rhfw::platform_bridge;

	::Windows::UI::Core::CoreWindow^ nativeWindow;

	WinstoreWindow(::Windows::UI::Core::CoreWindow^ window);
protected:
	virtual void showSoftKeyboardImpl(KeyboardType type) override;
	virtual void hideSoftKeyboardImpl() override;
public:
	~WinstoreWindow() {
	}
	WinstoreWindow* get() override {
		return this;
	}

	virtual void close() override;

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer) override;
	virtual void detachFromRenderer() override;
	virtual bool isHardwareKeyboardPresent() override;

	::Windows::UI::Core::CoreWindow^ getNativeWindow() {
		return nativeWindow;
	}

	virtual WindowStyle getWindowStyle() override;
	virtual void setWindowStyle(WindowStyle style) override;
	virtual bool supportsWindowStyle(WindowStyle style) override;
};
#pragma warning(pop)
} // namespace core
} // namespace rhfw

#endif /*WINSTORE_WINDOW_H_*/
