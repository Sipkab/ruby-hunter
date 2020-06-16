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
#ifndef TEST_SAPPHIRE_DIALOGS_ITEMS_KEYMAPDIALOGITEM_H_
#define TEST_SAPPHIRE_DIALOGS_ITEMS_KEYMAPDIALOGITEM_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <gen/types.h>

namespace userapp {
using namespace rhfw;

class KeyMapDialogItem: public DialogItem {
	FixedString name;
	KeyCode keyCode = KeyCode::KEY_UNKNOWN;
	const char* keyName = "";
	bool activated = false;
	InputDevice inputDevice;
protected:
	virtual void onHighlightRevoked() override;
public:
	KeyMapDialogItem(FixedString name, KeyCode keycode, InputDevice inputdevice);
	virtual bool onKeyEvent() override;

	virtual void onSelected(const Vector2F* pointer) override;

	virtual void draw(const rhfw::Matrix2D& mvp) override;
	virtual float measureTextWidth(float textsize, float maxwidth) override;
	virtual float measureTextHeight(float textsize, float maxwidth) override;

	KeyCode getKeyCode() const {
		return keyCode;
	}

};

} // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_ITEMS_KEYMAPDIALOGITEM_H_ */
