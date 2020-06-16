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
//  MacOsxWindow.cpp
//
//  Created by User on 2016. 03. 12..
//  Copyright © 2016. User. All rights reserved.
//

#include <macosxplatform/MacOsxNativeWindow.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSCursor.h>

#include <framework/core/Window.h>
#include <framework/render/Renderer.h>
#include <dispatch/dispatch.h>

namespace rhfw {
namespace core {

MacOsxWindow::~MacOsxWindow() {
}

void MacOsxWindow::setNativeWindow(macosx_native_window_cpp* nativewindow, core::time_micros time) {
	this->nativeWindow = nativewindow;
	nativewindow->window.appwindow = this;
	this->windowStyle = WindowStyle::NO_FLAG;
	auto style = nativewindow->window.styleMask;
	if (HAS_FLAG(style, NSFullScreenWindowMask)) {
		this->windowStyle |= WindowStyle::FULLSCREEN;
	}
	if (HAS_FLAG(style, NSTitledWindowMask)) {
		this->windowStyle |= WindowStyle::BORDERED;
	}
}

void MacOsxWindow::attachToRenderer(const Resource<render::Renderer>& renderer) {
	WindowBase::attachToRenderer(renderer);
	attachedRenderer->attachWindow(this);
}
void MacOsxWindow::detachFromRenderer() {
	attachedRenderer->detachWindow(this);
	WindowBase::detachFromRenderer();
}

void MacOsxWindow::showSoftKeyboardImpl(KeyboardType type) {
}

void MacOsxWindow::hideSoftKeyboardImpl() {
}

void MacOsxWindow::close() {
	MacOsxNativeWindow* window = this->nativeWindow->window;
	dispatch_async(dispatch_get_main_queue(), ^ {
			[window close];
		});
}

WindowStyle MacOsxWindow::getWindowStyle() {
	return windowStyle;
}
void MacOsxWindow::setWindowStyle(WindowStyle style) {
	ASSERT(supportsWindowStyle(style));
	WindowStyle changes = style ^ windowStyle;
	if (changes == WindowStyle::NO_FLAG) {
		return;
	}
	//FULLSCREEN is set later by glue message
	this->windowStyle = (style & ~WindowStyle::FULLSCREEN) | (this->windowStyle & WindowStyle::FULLSCREEN);
	MacOsxNativeWindow* window = this->nativeWindow->window;
	dispatch_async(dispatch_get_main_queue(), ^ {
		auto frame = window.frame;
		if (HAS_FLAG(style, WindowStyle::BORDERED)) {
			window.fullscreenOptions = NSApplicationPresentationFullScreen | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock;
			//NOTE: when our window display on the largest screen without border
			//(that is no FullScreen flags set, and the menu bar is visible)
			//then when setting NSTitledWindowMask, the title bar will not be visible, and the frame of the window will be under the menu bar
			//to work around this, set the frame to and arbitrary rect,
			//the old frame will be set back at the end of this dispatch block
			//[window setFrame: NSRect{100,100,100,100} display: NO];
			//window.styleMask = DEFAULT_WINDOW_STYLE;
			//window.styleMask = (window.styleMask | NSTitledWindowMask) & ~NSBorderlessWindowMask;
			if (!HAS_FLAG(changes, WindowStyle::FULLSCREEN) && HAS_FLAG(style, WindowStyle::FULLSCREEN) && HAS_FLAG(window.styleMask, NSFullScreenWindowMask)) {
				//fullscreen status did not change
				//we should be fullscreen
				//we are currently fullscreen
				//we should set the presentation options manually
				[[NSApplication sharedApplication] setPresentationOptions: window.fullscreenOptions];
			}
		} else {
			window.fullscreenOptions = NSApplicationPresentationFullScreen | NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock;
			//window.styleMask = NSBorderlessWindowMask;
			//window.styleMask = (window.styleMask | NSBorderlessWindowMask) & ~NSTitledWindowMask;
			if (!HAS_FLAG(changes, WindowStyle::FULLSCREEN) && HAS_FLAG(style, WindowStyle::FULLSCREEN) && HAS_FLAG(window.styleMask, NSFullScreenWindowMask)) {
				//fullscreen status did not change
				//we should be fullscreen
				//we are currently fullscreen
				//we should set the presentation options manually
				[[NSApplication sharedApplication] setPresentationOptions: window.fullscreenOptions];
			}
		}
		//[window setFrame: frame display: NO];
		if (HAS_FLAG(style, WindowStyle::FULLSCREEN)) {
			if(!HAS_FLAG(window.styleMask, NSFullScreenWindowMask)) {
				[window toggleFullScreen:nil];
			}
		} else {
			if(HAS_FLAG(window.styleMask, NSFullScreenWindowMask)) {
				[window toggleFullScreen:nil];
			}
		}
	});
}
bool MacOsxWindow::supportsWindowStyle(WindowStyle style) {
	//support everything
	return true;
}

void MacOsxWindow::setCursorVisible(bool visible){
	if(cursorVisible == visible){
		return;
	}
	this->cursorVisible = visible;
	if(visible){
		[NSCursor unhide];
	}else{
		[NSCursor hide];
	}
}

} // namespace core
} // namespace rhfw

