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
 * LevelDetailsLayer.h
 *
 *  Created on: 2016. apr. 24.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_LEVELDETAILSLAYER_H_
#define TEST_SAPPHIRE_DIALOGS_LEVELDETAILSLAYER_H_

#include <framework/geometry/Vector.h>
#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/level/Level.h>
#include <sapphire/community/CommunityConnection.h>

namespace userapp {
class LevelSelectorLayer;
class SapphireLevelDescriptor;
class LevelUploadDialogLayer;
class LevelDetailsLayer: public DialogLayer {
	friend class LevelUploadDialogLayer;
private:
	const SapphireLevelDescriptor* descriptor;
	Level level;
	LevelSelectorLayer* selectorLayer = nullptr;
	DialogItem* editempty = nullptr;
	DialogItem* edititem = nullptr;
	DialogItem* uploadItem = nullptr;

	void startUpload();

	void removeUploadItems();
public:
	LevelDetailsLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level);
	~LevelDetailsLayer();

	void setSelectorLayer(LevelSelectorLayer* selectorlayer) {
		this->selectorLayer = selectorlayer;
	}

	void addReportItem();
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_LEVELDETAILSLAYER_H_ */
