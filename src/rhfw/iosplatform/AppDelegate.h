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
//  AppDelegate.h
//  TestApp
//
//  Created by User on 2016. 03. 12..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef AppDelegate_h
#define AppDelegate_h

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/EAGLDrawable.h>

@interface MyNewWindow : UIWindow <UIKeyInput, UITextInputTraits>
@property rhfw::core::Window* appwindow;
@property(nonatomic) UIKeyboardType keyboardType;
@end

@interface MyNewEAGLLayer : CAEAGLLayer
@property rhfw::core::Window* appwindow;
@property struct ios_native_layer* nativeLayer;
@end

struct ios_native_layer {
	MyNewEAGLLayer* __unsafe_unretained layer;

	MyNewWindow* __unsafe_unretained window;
	bool isVisible = false;

	ios_native_layer(MyNewEAGLLayer* __unsafe_unretained layer = nullptr) : layer {layer} {
	}
};

#endif /* AppDelegate_h */
