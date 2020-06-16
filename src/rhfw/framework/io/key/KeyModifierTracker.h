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
 * KeyModifierTracker.h
 *
 *  Created on: 2017. jul. 26.
 *      Author: sipka
 */

#ifndef JNI_FRAMEWORK_IO_KEY_KEYMODIFIERTRACKER_H_
#define JNI_FRAMEWORK_IO_KEY_KEYMODIFIERTRACKER_H_

#include <gen/types.h>

namespace rhfw {

class KeyModifierTracker {
	rhfw::KeyModifiers modifiers;
public:
	void down(rhfw::KeyCode kc) {
		switch (kc) {
			case rhfw::KeyCode::KEY_SHIFT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_SHIFT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::LEFT_SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_SHIFT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_CTRL: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_CTRL: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::LEFT_CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_CTRL: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_ALT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::ALT);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_ALT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::LEFT_ALT);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_ALT: {
				SET_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_ALT);
				break;
			}
			default: {
				break;
			}
		}
	}
	void up(rhfw::KeyCode kc) {
		switch (kc) {
			case rhfw::KeyCode::KEY_SHIFT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_SHIFT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::LEFT_SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_SHIFT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_SHIFT);
				break;
			}
			case rhfw::KeyCode::KEY_CTRL: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_CTRL: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::LEFT_CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_CTRL: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_CTRL);
				break;
			}
			case rhfw::KeyCode::KEY_ALT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::ALT);
				break;
			}
			case rhfw::KeyCode::KEY_LEFT_ALT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::LEFT_ALT);
				break;
			}
			case rhfw::KeyCode::KEY_RIGHT_ALT: {
				CLEAR_FLAG(modifiers, rhfw::KeyModifiers::RIGHT_ALT);
				break;
			}
			default: {
				break;
			}
		}
	}

	operator rhfw::KeyModifiers() const {
		return modifiers;
	}
};

}  // namespace rhfw

#endif /* JNI_FRAMEWORK_IO_KEY_KEYMODIFIERTRACKER_H_ */
