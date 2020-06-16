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
 * LevelSounder.cpp
 *
 *  Created on: 2016. apr. 27.
 *      Author: sipka
 */

#include <framework/audio/descriptor/WavAudioDescriptor.h>
#include <framework/core/timing.h>
#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/utils/BasicListener.h>

#include <sapphire/SapphireScene.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelSounder.h>

#include <gen/assets.h>

namespace userapp {

#define LOAD_SOUND(sclip, asset) \
	soundClips[(unsigned int)(sclip)] = { audioManager->createSoundClip(), [](audio::SoundClip* clip) {\
		clip->setDescriptor(new audio::WavAudioDescriptor{new AssetFileDescriptor{ asset } });\
	} }

LevelSounder::LevelSounder(Level* level)
		: level(level) {
	for (auto&& vol : elementSoundVolumes) {
		vol = 100;
	}
}
LevelSounder::~LevelSounder() {
}

void LevelSounder::playSoundsForTurn() {
	if (soundVolume == 0 || !audioManager.isLoaded()) {
		return;
	}
	unsigned int soundcount = level->getSoundCount();
	for (unsigned int i = 0; i < soundcount; ++i) {
		auto& s = level->getGameSound(i);
		auto& clip = soundClips[(unsigned int) s.getSound()];
		auto& svol = elementSoundVolumes[(unsigned int) s.getSound()];
		//temporary
		if (svol == 0) {
			continue;
		}
		bool res = audioManager->playSingle(clip, soundVolume / 100.0f * svol / 100.0f);
		if (!res) {
			//failed to play this, do not try next
			break;
		}
	}
	if (level->getTurn() <= successOverTurn) {
		successOverTurn = 0;
	}

	if (successOverTurn > 0 && level->getTurn() - successOverTurn == 3 && level->getMinersTotal() > 0) {
		auto& svol = elementSoundVolumes[(unsigned int) SapphireSound::Win];
		if (svol > 0) {
			//level->getMinersTotal() > 0: if we are testing a level and no miner is present, then do not play win sound
			audioManager->playSingle(soundClips[(unsigned int) SapphireSound::Win], soundVolume / 100.0f * svol / 100.0f);
		}
	}
	if (successOverTurn == 0 && level->isSuccessfullyOver()) {
		successOverTurn = level->getTurn();
	}
}

void LevelSounder::levelReloaded() {
	successOverTurn = 0;
	if (level->getMusicName() != nullptr) {
		scene->setBackgroundMusic(level->getMusicName());
	}
}

void LevelSounder::attachToScene(SapphireScene* scene) {
	this->scene = scene;
	if (level->getMusicName() != nullptr) {
		scene->setBackgroundMusic(level->getMusicName());
	}
	scene->settingsChangedListeners += *this;
	onSettingsChanged(scene->getSettings());
}

void LevelSounder::onSettingsChanged(const SapphireSettings& settings) {
	if (soundVolume != settings.sound) {
		bool turned = (soundVolume == 0) != (settings.sound == 0);
		soundVolume = settings.sound;
		if (turned) {
			if (soundVolume > 0) {
				audioManager = userapp::audioManager;
				if (audioManager.isLoaded()) {
					LOAD_SOUND(SapphireSound::UseDoor, RAssets::gameres::game_sapphire::sound::usedoor_wav);
					LOAD_SOUND(SapphireSound::PickKey, RAssets::gameres::game_sapphire::sound::grabkey_wav);
					LOAD_SOUND(SapphireSound::PickEmerald, RAssets::gameres::game_sapphire::sound::grabemld_wav);
					LOAD_SOUND(SapphireSound::PickSapphire, RAssets::gameres::game_sapphire::sound::grabsphr_wav);
					LOAD_SOUND(SapphireSound::PickRuby, RAssets::gameres::game_sapphire::sound::grabruby_wav);
					LOAD_SOUND(SapphireSound::PickBomb, RAssets::gameres::game_sapphire::sound::grabbomb_wav);
					LOAD_SOUND(SapphireSound::Walk, RAssets::gameres::game_sapphire::sound::walk_wav);
					LOAD_SOUND(SapphireSound::Dig, RAssets::gameres::game_sapphire::sound::dig_wav);
					LOAD_SOUND(SapphireSound::YamYam, RAssets::gameres::game_sapphire::sound::yamyam_wav);
					LOAD_SOUND(SapphireSound::PutBomb, RAssets::gameres::game_sapphire::sound::setbomb_wav);
					LOAD_SOUND(SapphireSound::FallAcid, RAssets::gameres::game_sapphire::sound::acid_wav);
					LOAD_SOUND(SapphireSound::BagOpen, RAssets::gameres::game_sapphire::sound::bagopen_wav);
					LOAD_SOUND(SapphireSound::SwampMove, RAssets::gameres::game_sapphire::sound::swamp_wav);
					LOAD_SOUND(SapphireSound::MinerDie, RAssets::gameres::game_sapphire::sound::die_wav);
					LOAD_SOUND(SapphireSound::Explode, RAssets::gameres::game_sapphire::sound::explode_wav);
					LOAD_SOUND(SapphireSound::Laser, RAssets::gameres::game_sapphire::sound::laser_wav);
					LOAD_SOUND(SapphireSound::RobotMove, RAssets::gameres::game_sapphire::sound::robot_wav);
					LOAD_SOUND(SapphireSound::DropStill, RAssets::gameres::game_sapphire::sound::drop_wav);
					LOAD_SOUND(SapphireSound::Elevator, RAssets::gameres::game_sapphire::sound::elevator_wav);
					LOAD_SOUND(SapphireSound::SapphireBreak, RAssets::gameres::game_sapphire::sound::sphrbrk_wav);
					LOAD_SOUND(SapphireSound::PushRock, RAssets::gameres::game_sapphire::sound::push_wav);
					LOAD_SOUND(SapphireSound::PushBag, RAssets::gameres::game_sapphire::sound::pushbag_wav);
					LOAD_SOUND(SapphireSound::PushBomb, RAssets::gameres::game_sapphire::sound::pushbomb_wav);
					LOAD_SOUND(SapphireSound::PushSafe, RAssets::gameres::game_sapphire::sound::pushbox_wav);
					LOAD_SOUND(SapphireSound::PushCushion, RAssets::gameres::game_sapphire::sound::pcushion_wav);
					LOAD_SOUND(SapphireSound::ConvertBag, RAssets::gameres::game_sapphire::sound::bagconv_wav);
					LOAD_SOUND(SapphireSound::ConvertRuby, RAssets::gameres::game_sapphire::sound::rubyconv_wav);
					LOAD_SOUND(SapphireSound::ConvertEmerald, RAssets::gameres::game_sapphire::sound::emldconv_wav);
					LOAD_SOUND(SapphireSound::ConvertSapphire, RAssets::gameres::game_sapphire::sound::sphrconv_wav);
					LOAD_SOUND(SapphireSound::ConvertRock, RAssets::gameres::game_sapphire::sound::stnconv_wav);
					LOAD_SOUND(SapphireSound::FallBag, RAssets::gameres::game_sapphire::sound::bagfall_wav);
					LOAD_SOUND(SapphireSound::FallEmerald, RAssets::gameres::game_sapphire::sound::emldfall_wav);
					LOAD_SOUND(SapphireSound::FallRuby, RAssets::gameres::game_sapphire::sound::rubyfall_wav);
					LOAD_SOUND(SapphireSound::FallSapphire, RAssets::gameres::game_sapphire::sound::sphrfall_wav);
					LOAD_SOUND(SapphireSound::FallRockSoft, RAssets::gameres::game_sapphire::sound::stnfall_wav);
					LOAD_SOUND(SapphireSound::FallRockHard, RAssets::gameres::game_sapphire::sound::stnhard_wav);
					LOAD_SOUND(SapphireSound::FallCushion, RAssets::gameres::game_sapphire::sound::cushion_wav);
					LOAD_SOUND(SapphireSound::RollBagBomb, RAssets::gameres::game_sapphire::sound::pushbomb_wav);
					LOAD_SOUND(SapphireSound::RollEmerald, RAssets::gameres::game_sapphire::sound::emldroll_wav);
					LOAD_SOUND(SapphireSound::RollRuby, RAssets::gameres::game_sapphire::sound::rubyroll_wav);
					LOAD_SOUND(SapphireSound::RollSapphire, RAssets::gameres::game_sapphire::sound::sphrroll_wav);
					LOAD_SOUND(SapphireSound::RollRock, RAssets::gameres::game_sapphire::sound::stnroll_wav);
					LOAD_SOUND(SapphireSound::ExitOpen, RAssets::gameres::game_sapphire::sound::exitopen_wav);
					LOAD_SOUND(SapphireSound::ExitClose, RAssets::gameres::game_sapphire::sound::exitclos_wav);
					LOAD_SOUND(SapphireSound::Win, RAssets::gameres::game_sapphire::sound::win_wav);
					LOAD_SOUND(SapphireSound::GameLost, RAssets::gameres::game_sapphire::sound::lose_wav);
					LOAD_SOUND(SapphireSound::Wheel, RAssets::gameres::game_sapphire::sound::wheel_wav);
					LOAD_SOUND(SapphireSound::BombTick, RAssets::gameres::game_sapphire::sound::bombtick_wav);
					LOAD_SOUND(SapphireSound::BugMove, RAssets::gameres::game_sapphire::sound::bug_wav);
					LOAD_SOUND(SapphireSound::LorryMove, RAssets::gameres::game_sapphire::sound::lorry_wav);

					LOAD_SOUND(SapphireSound::CitrineShatter, RAssets::gameres::game_sapphire::sound::citrineshatter_wav);

					LOAD_SOUND(SapphireSound::CitrineBreak, RAssets::gameres::game_sapphire::sound::citrinebreak_wav);
					LOAD_SOUND(SapphireSound::ConvertCitrine, RAssets::gameres::game_sapphire::sound::citrineconvert_wav);
					LOAD_SOUND(SapphireSound::RollCitrine, RAssets::gameres::game_sapphire::sound::citrineroll_wav);
					LOAD_SOUND(SapphireSound::PickCitrine, RAssets::gameres::game_sapphire::sound::citrinepick_wav);

					LOAD_SOUND(SapphireSound::Clock, RAssets::gameres::game_sapphire::sound::clock_wav);
				}
			} else {
				for (unsigned int i = 0; i < (unsigned int) SapphireSound::_count_of_entries; ++i) {
					soundClips[i] = nullptr;
				}
				audioManager = nullptr;
			}
		}
	}

	elementSoundVolumes[(unsigned int) SapphireSound::BagOpen] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::BombTick] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::BugMove] = settings.soundEnemies;
	elementSoundVolumes[(unsigned int) SapphireSound::CitrineBreak] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::CitrineShatter] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertBag] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertCitrine] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertEmerald] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertRock] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertRuby] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::ConvertSapphire] = settings.soundConvert;
	elementSoundVolumes[(unsigned int) SapphireSound::Dig] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::DropStill] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::Elevator] = settings.soundWheel;
	elementSoundVolumes[(unsigned int) SapphireSound::Explode] = settings.soundExplosion;
	elementSoundVolumes[(unsigned int) SapphireSound::ExitOpen] = settings.soundDoorUsing;
	elementSoundVolumes[(unsigned int) SapphireSound::ExitClose] = settings.soundDoorUsing;
	elementSoundVolumes[(unsigned int) SapphireSound::FallAcid] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallBag] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallCushion] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallEmerald] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallRockHard] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallRockSoft] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallRuby] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::FallSapphire] = settings.soundFalling;
	elementSoundVolumes[(unsigned int) SapphireSound::Laser] = settings.soundLaser;
	elementSoundVolumes[(unsigned int) SapphireSound::LorryMove] = settings.soundEnemies;
	elementSoundVolumes[(unsigned int) SapphireSound::PickBomb] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PickCitrine] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PickEmerald] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PickKey] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PickRuby] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PickSapphire] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::PushBag] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::PushBomb] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::PushCushion] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::PushRock] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::PushSafe] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::PutBomb] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::RobotMove] = settings.soundEnemies;
	elementSoundVolumes[(unsigned int) SapphireSound::RollBagBomb] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::RollCitrine] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::RollEmerald] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::RollRock] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::RollRuby] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::RollSapphire] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::SapphireBreak] = settings.soundGems;
	elementSoundVolumes[(unsigned int) SapphireSound::SwampMove] = settings.soundEnemies;
	elementSoundVolumes[(unsigned int) SapphireSound::UseDoor] = settings.soundDoorUsing;
	elementSoundVolumes[(unsigned int) SapphireSound::Walk] = settings.soundPickUp;
	elementSoundVolumes[(unsigned int) SapphireSound::Wheel] = settings.soundWheel;
	elementSoundVolumes[(unsigned int) SapphireSound::Win] = settings.soundDoorUsing;
	elementSoundVolumes[(unsigned int) SapphireSound::YamYam] = settings.soundYamYam;

}

}  // namespace userapp

