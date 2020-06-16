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
 * AndroidWindow.cpp
 *
 *  Created on: 2016. febr. 27.
 *      Author: sipka
 */

#include <framework/core/Window.h>
#include <framework/render/Renderer.h>
#include <framework/threading/Thread.h>

#include <android/native_window.h>
#include <android/native_activity.h>
#include <jni.h>

#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {
namespace core {

AndroidWindow::~AndroidWindow() {
	if (isAttachedToRenderer()) {
		detachFromRenderer();
	}
}

void AndroidWindow::setNativeWindow(ANativeWindow* window, core::time_micros time) {
	if (this->window != nullptr) {
		if (isAttachedToRenderer()) {
			attachedRenderer->detachWindow(this);
		}
		ANativeWindow_release(this->window);
	}
	this->window = window;
	if (window == nullptr) {
		internalSetSize(0, 0);
	} else {
		ANativeWindow_acquire(this->window);
		//these are negative on error
		int32_t w = ANativeWindow_getWidth(window);
		int32_t h = ANativeWindow_getHeight(window);
		if (w <= 0 || h <= 0) {
			internalSetSize(0, 0);
		} else {
			internalSetSize((unsigned int) w, (unsigned int) h);
		}
		if (isAttachedToRenderer()) {
			attachedRenderer->attachWindow(this);
		}
	}

	WindowBase::setVisibleToUser(shouldBeVisibleToUser && hasNativeWindow(), time);
}

void AndroidWindow::close() {
	if (!finishing) {
		finishing = true;
		ANativeActivity_finish(activity);
	}
}

void AndroidWindow::showSoftKeyboardImpl(KeyboardType type) {
	JNIEnv& java = *Thread::getJniEnvForCurrentThread();
	jobject& activityobject = activity->clazz;
	jclass activityclass = java.GetObjectClass(activityobject);
	ASSERT(activityclass != nullptr) << "java callback failed";
	jmethodID method = java.GetMethodID(activityclass, "requestKeyboard", "(I)V");
	ASSERT(method != nullptr) << "java callback failed";

	int typeparam;
	switch (type) {
		case KeyboardType::NUMERIC: {
			typeparam = 1;
			break;
		}
		default: {
			typeparam = 0;
			break;
		}
	}

	java.CallVoidMethod(activityobject, method, typeparam);

	java.DeleteLocalRef(activityclass);
}

void AndroidWindow::setVisibleToUser(bool visible, core::time_micros time) {
	shouldBeVisibleToUser = visible;
	WindowBase::setVisibleToUser(visible && hasNativeWindow(), time);
}

void AndroidWindow::hideSoftKeyboardImpl() {
	JNIEnv& java = *Thread::getJniEnvForCurrentThread();
	jobject& activityobject = activity->clazz;
	jclass activityclass = java.GetObjectClass(activityobject);
	ASSERT(activityclass != nullptr) << "java callback failed";
	jmethodID method = java.GetMethodID(activityclass, "dismissKeyboard", "()V");
	ASSERT(method != nullptr) << "java callback failed";

	java.CallVoidMethod(activityobject, method);

	java.DeleteLocalRef(activityclass);
}
bool AndroidWindow::isHardwareKeyboardPresent() {
	return false;
}

void AndroidWindow::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	if (hasNativeWindow()) {
		attachedRenderer->attachWindow(this);
	}
}

void AndroidWindow::detachFromRenderer() {
	if (isAttachedToRenderer() && hasNativeWindow()) {
		attachedRenderer->detachWindow(this);
	}
	WindowBase::detachFromRenderer();
}

} //namespace core
} // namespace rhfw

