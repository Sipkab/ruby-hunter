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
 * SoundPlayer.cpp
 *
 *  Created on: 2016. maj. 8.
 *      Author: sipka
 */

#include <framework/audio/SoundPlayer.h>
#include <framework/audio/AudioManager.h>

namespace rhfw {
namespace audio {

void SoundPlayer::onPlayerStopped() {
	this->currentClip = nullptr;
	clipNode.removeLinkFromList();
	tokenNode.removeLinkFromList();
	manager.onSoundPlayerStopped(*this);

	auto* token = this->token.get();
	this->token.unlink();
	if (token != nullptr) {
		token->playerStopped();
	}
}

void SoundPlayer::setVolumeGain(float gain) {
	if (this->volumeGain == gain) {
		return;
	}
	if (setVolumeGainImpl(gain)) {
		this->volumeGain = gain;
	}
}
}  // namespace audio
}  // namespace rhfw

