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
 * Window.cpp
 *
 *  Created on: 2016. febr. 27.
 *      Author: sipka
 */

#include <framework/core/Window.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/render/Renderer.h>

namespace rhfw {
namespace core {

LinkedList<Window, false> WindowBase::windows;

WindowBase::WindowBase()
		: surface { static_cast<Window*>(this) } {
}
WindowBase::~WindowBase() {
	ASSERT(!LinkedNode < Window > ::isInList()) << "Window was not removed from windows list";

	//cant call detachFromRenderer here, because its virtual and derived class is already destructed
	ASSERT(!isAttachedToRenderer()) << "Window was not detached from renderer";

	TimeListener::unsubscribe();
}
void WindowBase::addWindow(Window& window, void* args) {
	if (static_cast<LinkedNode<Window>&>(window).isInList()) {
		return;
	}
	windows.addToEnd(window);

	for (auto&& l : WindowStateListener::foreach()) {
		l.onWindowCreated(window, args);
	}
}
void WindowBase::removeWindow(Window& window) {
	ASSERT(static_cast<LinkedNode<Window>&>(window).isInList());
	for (auto&& l : WindowStateListener::foreach()) {
		l.onWindowDestroyed(window);
	}
	static_cast<LinkedNode<Window>&>(window).removeLinkFromList();
}

void WindowBase::setSize(const WindowSize& size) {
	if (this->size == size || (size.pixelSize.width() == 0 || size.pixelSize.height() == 0)) {
		return;
	}

	LOGI()<< "Window size changing from: " << this->size.pixelSize << " dpi: " << this->size.dpi;
	LOGI()<< "Window size changed: " << size.pixelSize << " dpi: " << size.dpi;

	this->size = size;

	for (auto&& l : sizeListeners.foreach()) {
		l.onSizeChanged(static_cast<Window&>(*this), size);
	}
}

void WindowBase::dispatchTouchEvent() {
	TouchEventListener* listener = touchTarget.get();
	if (listener != nullptr) {
		if (listener->onTouchEvent()) {
			return;
		}
		listener = nullptr;
		//returned false
	}
	if (listener == nullptr) {
		for (auto& listener : touchListeners.foreach()) {
			if (listener.onTouchEvent()) {
				touchTarget.link(&listener);
				break;
			}
		}
	}
//	TouchEvent& event = TouchEvent::instance;
//	if (event.isJustDown()) {
//		touchTarget.unlink();
//
//		for (auto& listener : touchListeners.foreach()) {
//			if (listener.onTouchEvent()) {
//				touchTarget.link(&listener);
//				break;
//			}
//		}
//	} else {
//		TouchEventListener* listener = touchTarget.get();
//
//		if (listener != nullptr) {
//			if (!listener->onTouchEvent()) {
//				touchTarget.unlink();
//			}
//		}
//	}
}

bool WindowBase::dispatchKeyEvent() {
	KeyEventListener* listener = keyTarget.get();
	if (listener != nullptr) {
		return listener->onKeyEvent();
	} else {
		for (auto& listener : keyListeners.foreach()) {
			if (listener.onShouldTakeKeyInput()) {
				keyTarget.link(&listener);
				listener.onKeyInputChanged(true);
				LOGTRACE()<< "listener dispatch";
				return listener.onKeyEvent();
			}
		}
	}
	return false;
}

void WindowBase::takeKeyboardInput(LinkedNode<KeyEventListener>* listener) {
	ASSERT(listener != nullptr) << "listener is nullptr";
	ASSERT(listener->get() != nullptr) << "listener data is nullptr";

	KeyEventListener* current = keyTarget.get();
	if (current != nullptr) {
		current->onKeyInputChanged(false);
		keyTarget.unlink();
	}
	keyTarget.link(listener);
	listener->get()->onKeyInputChanged(true);
}

void WindowBase::setVisibleToUser(bool visible, core::time_micros time) {
	if (this->visibleToUser == visible)
		return;

	this->visibleToUser = visible;
	for (auto&& l : accesStateListeners.foreach()) {
		l.onVisibilityToUserChanged(static_cast<Window&>(*this), visible);
	}

	if (visible) {
		lastForegroundUpdate = time;
		GlobalMonotonicTimeListener::subscribeListener(*this);
	} else {
		TimeListener::unsubscribe();
		//GlobalMonotonicTimeListener::unsubscribeListener(*this);
		currentForegroundTime += time - lastForegroundUpdate;
	}
}
void WindowBase::setInputFocused(bool focused) {
	if (this->inputFocused == focused)
		return;

	this->inputFocused = focused;

	for (auto&& l : accesStateListeners.foreach()) {
		l.onInputFocusChanged(static_cast<Window&>(*this), focused);
	}

}

void WindowBase::onTimeChanged(const time_millis& time, const time_millis& previous) {
	if (visibleToUser) {
		ASSERT(time >= lastForegroundUpdate) << "last foreground update was after update time";

		core::time_micros prevtime = currentForegroundTime;

		currentForegroundTime += time - lastForegroundUpdate;

		lastForegroundUpdate = time;

		for (auto&& listener : foregroundTimeListeners.foreach()) {
			listener.onTimeChanged(currentForegroundTime, prevtime);
		}
	}
}

void WindowBase::draw() {
	if (drawListeners.isEmpty()) {
		return;
	}
	//ASSERT(isReadyToDraw(), "Window is not ready to draw");
	auto* renderer = surface.getRenderer();
	renderer->pushRenderTarget(static_cast<Window*>(this));
	renderer->setViewPort(render::ViewPort { size.pixelSize });
	for (auto& listener : drawListeners.foreach()) {
		listener.onDraw();
	}
	renderer->popRenderTarget();
}

void WindowBase::attachToRenderer(const Resource<render::Renderer>& renderer) {
	ASSERT(!isAttachedToRenderer()) << "Window is already attached to renderer";

	this->attachedRenderer = renderer;
}

void WindowBase::detachFromRenderer() {
	this->attachedRenderer = Resource<render::Renderer> { };
}

void WindowBase::requestSoftKeyboard(KeyboardType type) {
	if (softKeyboardType != type) {
		showSoftKeyboardImpl(type);
		softKeyboardType = type;
	}
}

void WindowBase::dismissSoftKeyboard() {
	hideSoftKeyboardImpl();
	softKeyboardType = KeyboardType::NONE;
}

void WindowRenderSurface::set(render::Renderer* renderer, const Resource<render::RenderTarget>& rendertarget) {
	ASSERT(renderer != nullptr && rendertarget != nullptr);
	this->renderer = renderer;
	this->renderTarget = rendertarget;
}

} // namespace core
} // namespace rhfw

