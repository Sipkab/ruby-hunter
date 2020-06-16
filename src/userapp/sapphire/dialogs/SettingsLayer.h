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
 * SettingsLayer.h
 *
 *  Created on: 2016. apr. 17.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SETTINGSLAYER_H_
#define TEST_SAPPHIRE_SETTINGSLAYER_H_

#include <sapphire/dialogs/DialogLayer.h>
#include <sapphire/SapphireScene.h>

namespace userapp {
class AdvancedSoundDialog;

class SettingsLayer: public DialogLayer, private GamePadStateListener {
	friend class AdvancedSoundDialog;
private:
	virtual void onGamePadAttached(GamePad* gamepad) override;
	virtual void onGamePadDetached(GamePad* gamepad) override;
protected:
	SapphireSettings settings;

	const char* displayModeLabels[(unsigned int) SapphireFullScreenState::_count_of_entries];
	SapphireFullScreenState displayModeIndexMap[(unsigned int) SapphireFullScreenState::_count_of_entries];
	unsigned int displayModeCount = 0;

	const char* antiAliasLabels = nullptr;
	const char** antiAliasLabelMap = nullptr;

//	DialogItem* sendFeedbackItem = nullptr;
	DialogItem* gamePadControlsItem = nullptr;
	DialogItem* gameScaleItem = nullptr;

	void sendFeedback(DialogLayer* parent, DialogItem* fbitem, const char* content, unsigned int length);
public:
	SettingsLayer(SapphireUILayer* parent, const SapphireSettings& settings);
	~SettingsLayer();
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SETTINGSLAYER_H_ */
