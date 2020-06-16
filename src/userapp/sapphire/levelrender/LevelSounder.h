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
 * LevelSounder.h
 *
 *  Created on: 2016. apr. 27.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_LEVELSOUNDER_H_
#define TEST_SAPPHIRE_LEVEL_LEVELSOUNDER_H_

#include <framework/audio/AudioManager.h>
#include <framework/audio/SoundClip.h>
#include <framework/resource/Resource.h>

#include <appmain.h>
#include <sapphire/SapphireScene.h>

#include <gen/types.h>

namespace userapp {
class Level;
class SapphireScene;

using namespace rhfw;

class LevelSounder: public SapphireScene::SettinsChangedListener {
private:
	AutoResource<audio::AudioManager> audioManager;
	Level* level;
	unsigned int soundVolume = 0;
	SapphireScene* scene = nullptr;

	AutoResource<audio::SoundClip> soundClips[(unsigned int) SapphireSound::_count_of_entries];

	unsigned int elementSoundVolumes[(unsigned int) SapphireSound::_count_of_entries];

	unsigned int successOverTurn = 0;

public:
	LevelSounder(Level* level);
	~LevelSounder();

	void playSoundsForTurn();

	void levelReloaded();

	void attachToScene(SapphireScene* scene);

	virtual void onSettingsChanged(const SapphireSettings& settings) override;
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_LEVELSOUNDER_H_ */
