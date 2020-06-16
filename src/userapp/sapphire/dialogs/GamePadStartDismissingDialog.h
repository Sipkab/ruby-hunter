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
 * GamePadStartDismissingDialog.h
 *
 *  Created on: 2017. aug. 22.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_GAMEPADSTARTDISMISSINGDIALOG_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_GAMEPADSTARTDISMISSINGDIALOG_H_

#include <framework/io/key/KeyEvent.h>
#include <gen/types.h>
#include <sapphire/dialogs/DialogLayer.h>

namespace userapp {
using namespace rhfw;

class GamePadStartDismissingDialog: public DialogLayer {
public:
	using DialogLayer::DialogLayer;
	virtual bool onKeyEventImpl() override {
		if (KeyEvent::instance.getAction() == KeyAction::DOWN && KeyEvent::instance.getKeycode() == KeyCode::KEY_GAMEPAD_START) {
			onBackRequested();
			return true;
		}
		return DialogLayer::onKeyEventImpl();
	}
};

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_GAMEPADSTARTDISMISSINGDIALOG_H_ */
