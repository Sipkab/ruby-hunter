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
#ifndef _TEST_SAPPHIRE_DIALOGS_KEYMAPDIALOG_H_
#define _TEST_SAPPHIRE_DIALOGS_KEYMAPDIALOG_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/SapphireScene.h>

namespace userapp {

class KeyMapDialog: public DialogLayer {
protected:
	SapphireKeyMap keyMap;
public:
	KeyMapDialog(SapphireUILayer* parent, const SapphireKeyMap& keymap, InputDevice inputdevice);
	~KeyMapDialog();
};

} // namespace userapp

#endif /* _TEST_SAPPHIRE_DIALOGS_KEYMAPDIALOG_H_ */
