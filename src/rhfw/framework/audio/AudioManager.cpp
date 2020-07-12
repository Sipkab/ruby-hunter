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
 * AudioManager.cpp
 *
 *  Created on: 2016. marc. 2.
 *      Author: sipka
 */

#include <framework/audio/AudioManager.h>
#include <framework/audio/SoundClip.h>
#include <framework/audio/SoundPlayer.h>

namespace rhfw {
namespace audio {

SoundPlayerToken AudioManager::playSingle(SoundClip& clip, float volumegain) {
	SoundPlayer* player = nullptr;
	for (auto* p : stoppedPlayers.pointers().reverse()) {
		if (p->getFormat() == clip.getFormat()) {
			//LOGV("Reusing stopped SoundPlayer");
			player = p;
			player->removeLinkFromList();
			goto found_player;
		}
	}
	player = createSoundPlayerImpl(clip.getFormat());
	//LOGV("Create SoundPlayer with format channels: %hu sampleRate: %u bitsPerSample: %hu blockAlign: %hu endianness: %s",
	//	clip.getFormat().channels, clip.getFormat().sampleRate, clip.getFormat().bitsPerSample, clip.getFormat().blockAlign,
	//TOSTRING(clip.getFormat().endian));
	if (player == nullptr) {
		LOGW()<< "Failed to create new SoundPlayer (Out of available sound resources?)";
		//did not find player with suitable format, try to take one based on priority
		if (clip.getPriority() != AudioPriority::LOW) {
			//not taking any player on lowest priority
			for (AudioPriority pri = AudioPriority::MEDIUM; pri != clip.getPriority(); pri = (AudioPriority) ((int) pri + 1)) {
				for (auto* p : playingPlayersByPriority[(unsigned int) pri].pointers()) {
					if (p->getFormat() == clip.getFormat()) {
						player = p;
						LOGV() << "Taking Sound player from lower priority. " << pri << " -> " << clip.getPriority();
						goto after_outerloop;
					}
				}
			}
		}
		//if we are here, we failed to take a player
		LOGW() << "Failed to start playing sound, no player available with lower priority";
		return SoundPlayerToken {};

		after_outerloop:

		player->clipNode.removeLinkFromList();
		player->removeLinkFromList();
		if(player->stopAndPlay(clip)) {
			player->setVolumeGain(volumegain);
			goto after_playing;
		} else {
			//failed to start clip
			stoppedPlayers.addToEnd(*player);
			return SoundPlayerToken {};
		}
	}
	found_player:

	player->setVolumeGain(volumegain);
	if (!player->play(clip)) {
		//failed to start clip with player
		stoppedPlayers.addToEnd(*player);
		return SoundPlayerToken { };
	}

	after_playing:

	clip.dependentPlayers.addToEnd(player->clipNode);
	player->currentClip = &clip;
	playingPlayersByPriority[(unsigned int) clip.getPriority()].addToEnd(*player);

	SoundPlayerToken result { player->tokenNode };
	player->token.link(result);
	return util::move(result);
}

void AudioManager::onSoundPlayerStopped(SoundPlayer& player) {
	LOGTRACE();
	player.removeLinkFromList();
	stoppedPlayers.addToEnd(player);
}

bool AudioManager::load() {
	return loadImpl();
}

void AudioManager::free() {
	for (auto&& list : playingPlayersByPriority) {
		for (auto&& node : list.pointers()) {
			delete node;
		}
	}
	for (auto&& node : stoppedPlayers.pointers()) {
		delete node;
	}
	freeImpl();
}

}  // namespace audio
}  // namespace rhfw

