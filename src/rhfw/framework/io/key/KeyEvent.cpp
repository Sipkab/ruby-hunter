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
 * KeyEvent.cpp
 *
 *  Created on: 2015 aug. 28
 *      Author: sipka
 */

#include <framework/io/key/KeyEvent.h>

namespace rhfw {

KeyEvent KeyEvent::instance;

bool KeyEvent::dispatchMessage(KeyMessage* msg) {
	this->current = msg;

	return msg->window->dispatchKeyEvent();
}
}
