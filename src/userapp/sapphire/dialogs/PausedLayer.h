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
 * PausedLayer.h
 *
 *  Created on: 2016. apr. 27.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_PAUSEDLAYER_H_
#define TEST_SAPPHIRE_DIALOGS_PAUSEDLAYER_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/dialogs/GamePadStartDismissingDialog.h>

namespace userapp {
class PlayerLayer;
} // namespace userapp

namespace userapp {

class PausedLayer: public GamePadStartDismissingDialog {
private:

public:
	PausedLayer(PlayerLayer* parent);

};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_PAUSEDLAYER_H_ */
