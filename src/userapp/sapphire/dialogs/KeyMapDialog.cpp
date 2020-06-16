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
#include <sapphire/dialogs/KeyMapDialog.h>
#include <sapphire/dialogs/items/KeyMapDialogItem.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {
using namespace rhfw;

KeyMapDialog::KeyMapDialog(SapphireUILayer* parent, const SapphireKeyMap& keymap, InputDevice inputdevice)
		: DialogLayer(parent), keyMap(keymap) {
	KeyMapDialogItem* p1left;
	KeyMapDialogItem* p1right;
	KeyMapDialogItem* p1up;
	KeyMapDialogItem* p1down;
	KeyMapDialogItem* p1bomb;
	KeyMapDialogItem* p1pick;

	if (inputdevice == InputDevice::GAMEPAD) {
		addDialogItem(new TextDialogItem("Help: Try toggling the ANALOG switch on older controllers."));
		addDialogItem(new EmptyDialogItem(0.5f));
	}

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	KeyMapDialogItem* p2left;
	KeyMapDialogItem* p2right;
	KeyMapDialogItem* p2up;
	KeyMapDialogItem* p2down;
	KeyMapDialogItem* p2bomb;
	KeyMapDialogItem* p2pick;

	p1left = new KeyMapDialogItem("Player 1 left", keyMap[SapphireKeyCode::KEY_P1_LEFT], inputdevice);
	p1right = new KeyMapDialogItem("Player 1 right", keyMap[SapphireKeyCode::KEY_P1_RIGHT], inputdevice);
	p1up = new KeyMapDialogItem("Player 1 up", keyMap[SapphireKeyCode::KEY_P1_UP], inputdevice);
	p1down = new KeyMapDialogItem("Player 1 down", keyMap[SapphireKeyCode::KEY_P1_DOWN], inputdevice);
	p1bomb = new KeyMapDialogItem("Player 1 bomb", keyMap[SapphireKeyCode::KEY_P1_BOMB], inputdevice);
	p1pick = new KeyMapDialogItem("Player 1 pick", keyMap[SapphireKeyCode::KEY_P1_PICK], inputdevice);

	p2left = new KeyMapDialogItem("Player 2 left", keyMap[SapphireKeyCode::KEY_P2_LEFT], inputdevice);
	p2right = new KeyMapDialogItem("Player 2 right", keyMap[SapphireKeyCode::KEY_P2_RIGHT], inputdevice);
	p2up = new KeyMapDialogItem("Player 2 up", keyMap[SapphireKeyCode::KEY_P2_UP], inputdevice);
	p2down = new KeyMapDialogItem("Player 2 down", keyMap[SapphireKeyCode::KEY_P2_DOWN], inputdevice);
	p2bomb = new KeyMapDialogItem("Player 2 bomb", keyMap[SapphireKeyCode::KEY_P2_BOMB], inputdevice);
	p2pick = new KeyMapDialogItem("Player 2 pick", keyMap[SapphireKeyCode::KEY_P2_PICK], inputdevice);

	addDialogItem(p1left);
	addDialogItem(p1right);
	addDialogItem(p1up);
	addDialogItem(p1down);
	addDialogItem(p1bomb);
	addDialogItem(p1pick);

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(p2left);
	addDialogItem(p2right);
	addDialogItem(p2up);
	addDialogItem(p2down);
	addDialogItem(p2bomb);
	addDialogItem(p2pick);
#else
	p1left = new KeyMapDialogItem("Move left", keyMap[SapphireKeyCode::KEY_P1_LEFT], inputdevice);
	p1right = new KeyMapDialogItem("Move right", keyMap[SapphireKeyCode::KEY_P1_RIGHT], inputdevice);
	p1up = new KeyMapDialogItem("Move up", keyMap[SapphireKeyCode::KEY_P1_UP], inputdevice);
	p1down = new KeyMapDialogItem("Move down", keyMap[SapphireKeyCode::KEY_P1_DOWN], inputdevice);
	p1bomb = new KeyMapDialogItem("Place bomb", keyMap[SapphireKeyCode::KEY_P1_BOMB], inputdevice);
	p1pick = new KeyMapDialogItem("Pick", keyMap[SapphireKeyCode::KEY_P1_PICK], inputdevice);

	addDialogItem(p1left);
	addDialogItem(p1right);
	addDialogItem(p1up);
	addDialogItem(p1down);
	addDialogItem(p1bomb);
	addDialogItem(p1pick);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

	KeyMapDialogItem* editorpaint = new KeyMapDialogItem("Hold to paint in editor", keyMap[SapphireKeyCode::KEY_EDITOR_PAINT], inputdevice);
	KeyMapDialogItem* resetlevel = new KeyMapDialogItem("Reset level", keyMap[SapphireKeyCode::KEY_RESET_LEVEL], inputdevice);
	KeyMapDialogItem* quicksuspend = new KeyMapDialogItem("Suspend & continue level", keyMap[SapphireKeyCode::KEY_QUICK_SUSPEND],
			inputdevice);

	KeyMapDialogItem* increasespeed = new KeyMapDialogItem("Increase speed", keyMap[SapphireKeyCode::KEY_INCREASE_SPEED], inputdevice);
	KeyMapDialogItem* decreasespeed = new KeyMapDialogItem("Decrease speed", keyMap[SapphireKeyCode::KEY_DECREASE_SPEED], inputdevice);

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(editorpaint);
	addDialogItem(resetlevel);
	addDialogItem(quicksuspend);

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(increasespeed);
	addDialogItem(decreasespeed);

	addDialogItem(new EmptyDialogItem(0.5f));

	addDialogItem(new CommandDialogItem("Apply", [=] {
		keyMap.set(SapphireKeyCode::KEY_P1_LEFT, p1left->getKeyCode());
		keyMap.set(SapphireKeyCode::KEY_P1_RIGHT, p1right->getKeyCode());
		keyMap.set(SapphireKeyCode::KEY_P1_UP, p1up->getKeyCode());
		keyMap.set(SapphireKeyCode::KEY_P1_DOWN, p1down->getKeyCode());
		keyMap.set(SapphireKeyCode::KEY_P1_BOMB, p1bomb->getKeyCode());
		keyMap.set(SapphireKeyCode::KEY_P1_PICK, p1pick->getKeyCode());
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
			keyMap.set(SapphireKeyCode::KEY_P2_LEFT, p2left->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_P2_RIGHT, p2right->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_P2_UP, p2up->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_P2_DOWN, p2down->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_P2_BOMB, p2bomb->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_P2_PICK, p2pick->getKeyCode());
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

			keyMap.set(SapphireKeyCode::KEY_EDITOR_PAINT, editorpaint->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_RESET_LEVEL, resetlevel->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_QUICK_SUSPEND, quicksuspend->getKeyCode());

			keyMap.set(SapphireKeyCode::KEY_INCREASE_SPEED, increasespeed->getKeyCode());
			keyMap.set(SapphireKeyCode::KEY_DECREASE_SPEED, decreasespeed->getKeyCode());

			auto* ss = static_cast<SapphireScene*>(getScene());
			if (inputdevice == InputDevice::GAMEPAD) {
				ss->setGamePadKeyMap(keyMap);
			} else {
				ss->setKeyMap(keyMap);
			}
			dismiss();
		}));
	addDialogItem(new CommandDialogItem("Cancel", [=] {
		dismiss();
	}));

}
KeyMapDialog::~KeyMapDialog() {
}

} // namespace userapp
