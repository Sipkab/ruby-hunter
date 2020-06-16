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
 * LevelSuccessLayer.h
 *
 *  Created on: 2016. apr. 28.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_LEVELSUCCESSLAYER_H_
#define TEST_SAPPHIRE_DIALOGS_LEVELSUCCESSLAYER_H_

#include <framework/geometry/Vector.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <framework/utils/FixedString.h>

namespace userapp {
class PlayerLayer;
} // namespace userapp

namespace userapp {
class SapphireLevelDescriptor;
class LevelSuccessLayer: public DialogLayer {
private:
	const SapphireLevelDescriptor* next;
public:
	LevelSuccessLayer(PlayerLayer* parent, const SapphireLevelDescriptor* next);

	virtual void dismiss() override;

	virtual void setScene(Scene* scene) override;
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_LEVELSUCCESSLAYER_H_ */
