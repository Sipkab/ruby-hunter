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
#include <framework/core/Window.h>
#include <framework/render/Renderer.h>

namespace rhfw {
namespace core {

WinstoreWindow::WinstoreWindow(::Windows::UI::Core::CoreWindow^ window)
	: nativeWindow {window} {
}

void WinstoreWindow::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	attachedRenderer->attachWindow(this);
}
void WinstoreWindow::detachFromRenderer() {
	if (isAttachedToRenderer()) {
		attachedRenderer->detachWindow(this);
	}
	WindowBase::detachFromRenderer();
}

void WinstoreWindow::showSoftKeyboardImpl(KeyboardType type) {
	Windows::UI::ViewManagement::InputPane::GetForCurrentView()->TryShow();
}
void WinstoreWindow::hideSoftKeyboardImpl() {
	Windows::UI::ViewManagement::InputPane::GetForCurrentView()->TryHide();
}

bool WinstoreWindow::isHardwareKeyboardPresent() {
	Windows::Devices::Input::KeyboardCapabilities^ cap = ref new Windows::Devices::Input::KeyboardCapabilities();
	return cap->KeyboardPresent;
}

void WinstoreWindow::close() {
	//TODO
	::Windows::ApplicationModel::Core::CoreApplication::Exit();
	//nativeWindow->Close();
}

//TODO proper style handling
WindowStyle WinstoreWindow::getWindowStyle() {
	return WindowStyle::FULLSCREEN;
}
void WinstoreWindow::setWindowStyle(WindowStyle style) {
	ASSERT(supportsWindowStyle(style));
}
bool WinstoreWindow::supportsWindowStyle(WindowStyle style) {
	return style == WindowStyle::FULLSCREEN;
}

} //namespace core
} //namespace rhfw

