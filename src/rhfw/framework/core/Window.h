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
 * Window.h
 *
 *  Created on: 2016. febr. 26.
 *      Author: sipka
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <framework/utils/LifeCycleChain.h>
#include <framework/utils/BasicGlobalListener.h>
#include <framework/utils/BasicListener.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LinkedNode.h>
#include <framework/resource/Resource.h>
#include <framework/geometry/Vector.h>
#include <framework/core/timing.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/types.h>

namespace rhfw {
namespace render {
class Renderer;
class RenderTarget;
}  // namespace render

class platform_bridge;
namespace core {

//making this class not exact implemented will result breaking listener callback methods
typedef class WINDOW_EXACT_CLASS_TYPE Window;

class WindowSize {
public:
	Size2UI pixelSize;
	Vector2F dpi;

	WindowSize()
			: pixelSize { 0, 0 }, dpi { 0.0f, 0.0f } {
	}
	WindowSize(const Size2UI& pixelsize, const Vector2F& dpi)
			: pixelSize { pixelsize }, dpi { dpi } {
	}

	bool operator==(const WindowSize& o) const {
		return pixelSize == o.pixelSize && dpi == o.dpi;
	}
	bool operator!=(const WindowSize& o) const {
		return pixelSize != o.pixelSize || dpi != o.dpi;
	}

	//TODO introduce metric classes
	//in centimeters
	float getPhysicalWidth() const {
		return pixelSize.width() / dpi.x() * 2.54f;
	}
	//in centimeters
	float getPhysicalHeight() const {
		return pixelSize.height() / dpi.y() * 2.54f;
	}
	Vector2F getPhysicalSize() const {
		return pixelSize / dpi * 2.54f;
	}
	Size2F toPixels(const Vector2F& cm) const {
		return cm * dpi / 2.54f;
	}
	float toPixelsX(float cm) const {
		return cm * dpi.x() / 2.54f;
	}
	float toPixelsY(float cm) const {
		return cm * dpi.y() / 2.54f;
	}
	Size2F toCentiMeters(const Vector2F& px) const {
		return px / dpi * 2.54f;
	}
	float toCentiMetersX(float px) const {
		return px / dpi.x() * 2.54f;
	}
	float toCentiMetersY(float px) const {
		return px / dpi.y() * 2.54f;
	}

	float ratio() const {
		return (float) pixelSize.width() / pixelSize.height();
	}

	bool operator==(const WindowSize& o) {
		return pixelSize == o.pixelSize && dpi == o.dpi;
	}
	bool operator!=(const WindowSize& o) {
		return pixelSize != o.pixelSize || dpi != o.dpi;
	}
};

class WindowStateListener: public BasicGlobalListener<WindowStateListener> {
private:
public:
	virtual void onWindowCreated(Window& window, void* args) {
	}

	virtual void onWindowDestroyed(Window& window) {
	}
};
class WindowSizeListener: public BasicListener<WindowSizeListener> {
private:
public:

	virtual void onSizeChanged(Window& window, const WindowSize& size) = 0;
};
class DrawListener: public BasicListener<DrawListener> {
public:
	virtual void onDraw() {
	}
};
class TouchEventListener: public BasicListener<TouchEventListener> {
public:
	/**
	 * Event data is contained in object TouchEvent::instance
	 * Return true if wants to receive following events as well
	 */
	virtual bool onTouchEvent() {
		return false;
	}
};
class KeyEventListener: public BasicListener<KeyEventListener> {
public:
	/**
	 * Event data is contained in object KeyEvent::instance
	 * Return true if event was handled
	 */
	virtual bool onKeyEvent() {
		return false;
	}
	/**
	 * Called when the keyboard input changed.
	 * hasInput: true if the following key events will go to this instance
	 */
	virtual void onKeyInputChanged(bool hasInput) {
	}
	/**
	 * Called when no KeyEventListener is registered to take input.
	 * Return true to get registered, and receive the following onKeyEvent() calls.
	 * onKeyInputChanged will be called with true.
	 */
	virtual bool onShouldTakeKeyInput() {
		return false;
	}
};

class WindowAccesStateListener: public BasicListener<WindowAccesStateListener> {
private:
public:
	virtual void onVisibilityToUserChanged(Window& window, bool visible) {
	}
	virtual void onInputFocusChanged(Window& window, bool inputFocused) {
	}
};

class WindowRenderSurface: public LinkedNode<core::Window> {
private:
	render::Renderer* renderer = nullptr;
	Resource<render::RenderTarget> renderTarget = nullptr;
	core::Window* window;
public:
	WindowRenderSurface(core::Window* window)
			: window { window } {
	}

	void set(render::Renderer* renderer, const Resource<render::RenderTarget>& rendertarget);
	void reset() {
		this->renderer = nullptr;
		this->renderTarget = nullptr;
	}
	core::Window* get() override {
		return window;
	}
	render::Renderer* getRenderer() {
		return renderer;
	}
	Resource<render::RenderTarget> getRenderTarget() {
		return renderTarget;
	}

	bool isAttached() const {
		return renderer != nullptr;
	}
};

class WindowBase: public LinkedNode<Window>, public TimeListener {
private:
	friend class ::rhfw::platform_bridge;
	friend class render::Renderer;

	static LinkedList<Window, false> windows;

	static void addWindow(Window& window, void* args);
	static void removeWindow(Window& window);

	void* tag = nullptr;

	bool visibleToUser = false;
	bool inputFocused = false;

	LifeCycleChain<TouchEventListener, false> touchTarget;
	LifeCycleChain<KeyEventListener, false> keyTarget;

	core::time_micros lastForegroundUpdate;
	core::time_micros currentForegroundTime;

	WindowRenderSurface surface;

	KeyboardType softKeyboardType = KeyboardType::NONE;
protected:
	Resource<render::Renderer> attachedRenderer;
	WindowSize size;

	virtual void setVisibleToUser(bool visible, core::time_micros time);
	void setInputFocused(bool focused);

	void setSize(const WindowSize& size);

	virtual void showSoftKeyboardImpl(KeyboardType type) = 0;
	virtual void hideSoftKeyboardImpl() = 0;

	WindowBase();
public:
	static bool hasWindows() {
		return !windows.isEmpty();
	}

	static LinkedList<Window, false>& getWindows() {
		return windows;
	}

	WindowSizeListener::Events sizeListeners;
	DrawListener::Events drawListeners;
	TouchEventListener::Events touchListeners;
	KeyEventListener::Events keyListeners;
	WindowAccesStateListener::Events accesStateListeners;
	BasicListener<TimeListener>::Events foregroundTimeListeners;

	~WindowBase();

	core::time_micros getWindowForegroundTime() const {
		return currentForegroundTime;
	}

	virtual void onTimeChanged(const time_millis& time, const time_millis& previous) override;

	void draw();

	virtual bool isReadyToDraw() {
		return isVisibleToUser() && surface.isAttached();
	}

	WindowSize getWindowSize() const {
		//TODO check if main thread
		//return copy of size on main thread, not reference
		return size;
	}

	bool isInputFocused() const {
		return inputFocused;
	}
	bool isVisibleToUser() const {
		return visibleToUser;
	}

	void* getTag() {
		return tag;
	}
	void setTag(void* tag) {
		this->tag = tag;
	}

	WindowRenderSurface& getRenderSurface() {
		return surface;
	}

	void dispatchTouchEvent();
	bool dispatchKeyEvent();

	void takeKeyboardInput(LinkedNode<KeyEventListener>* listener);
	LinkedNode<KeyEventListener>* getKeyboardTarget() {
		return keyTarget.get();
	}

	virtual void close() = 0;

	void requestSoftKeyboard(KeyboardType type);
	void dismissSoftKeyboard();
	bool isSoftKeyboardShowing() const {
		return softKeyboardType != KeyboardType::NONE;
	}
	virtual bool isHardwareKeyboardPresent() = 0;

	virtual void attachToRenderer(const Resource<render::Renderer>& renderer);
	virtual void detachFromRenderer();

	bool isAttachedToRenderer() const {
		return attachedRenderer != nullptr;
	}

	virtual WindowStyle getWindowStyle() = 0;
	virtual void setWindowStyle(WindowStyle style) = 0;
	virtual bool supportsWindowStyle(WindowStyle style) = 0;
};

}  // namespace core
}  // namespace rhfw

#include WINDOW_EXACT_CLASS_INCLUDE

#endif /* WINDOW_H_ */
