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
//  IosWindow.cpp
//  TestApp
//
//  Created by User on 2016. 03. 12..
//  Copyright © 2016. User. All rights reserved.
//

#include <framework/core/Window.h>
#include <framework/render/Renderer.h>
#include <iosplatform/AppDelegate.h>
#include <dispatch/dispatch.h>

namespace rhfw {
namespace core {

void IosWindow::setNativeLayer(struct ios_native_layer* nativeLayer, core::time_micros time) {
	this->nativeLayer = nativeLayer;
	if (isAttachedToRenderer()) {
		if (hasNativeLayer()) {
			attachedRenderer->attachWindow(this);
		} else {
			attachedRenderer->detachWindow(this);
		}
	}
}

void IosWindow::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	if (hasNativeLayer()) {
		attachedRenderer->attachWindow(this);
	}
}

void IosWindow::detachFromRenderer() {
	if (hasNativeLayer()) {
		attachedRenderer->detachWindow(this);
	}
	WindowBase::detachFromRenderer();
}

void IosWindow::showSoftKeyboardImpl(KeyboardType type) {
	MyNewWindow* window = nativeLayer->window;
	dispatch_async(dispatch_get_main_queue(), ^{
		switch (type) {
			case KeyboardType::NUMERIC: {
				window.keyboardType = UIKeyboardTypeDecimalPad;
				break;
			}
			default: {
				window.keyboardType = UIKeyboardTypeASCIICapable;
				break;
			}
		}
		[window becomeFirstResponder];
	});
}

void IosWindow::hideSoftKeyboardImpl() {
	MyNewWindow* window = nativeLayer->window;
	dispatch_async(dispatch_get_main_queue(), ^{
		[window resignFirstResponder];
	});
}

} // namespace core
} // namespace rhfw

