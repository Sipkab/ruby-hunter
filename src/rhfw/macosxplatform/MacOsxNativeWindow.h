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
 * MacOsxNativeWindow.h
 *
 *  Created on: 2016. aug. 30.
 *      Author: sipka
 */

#ifndef MACOSXPLATFORM_MACOSXNATIVEWINDOW_H_
#define MACOSXPLATFORM_MACOSXNATIVEWINDOW_H_

#include <framework/core/Window.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSView.h>
#import <AppKit/NSTextInputContext.h>

#define DEFAULT_WINDOW_STYLE (NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask)

@class MacOsxNativeWindow;

class macosx_native_window_cpp {
public:
	MacOsxNativeWindow* window;
};

@interface MacOsxNativeWindow : NSWindow<NSWindowDelegate, NSTextInputClient>
@property NSView* view;
@property rhfw::core::Window* appwindow;
@property macosx_native_window_cpp* nativewindow_cpp;
@property NSApplicationPresentationOptions fullscreenOptions;

- (instancetype)initWithContentRect:(NSRect)contentRect;

@end

#endif /* MACOSXPLATFORM_MACOSXNATIVEWINDOW_H_ */
