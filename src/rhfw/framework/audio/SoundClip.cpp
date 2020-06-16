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
 * SoundClip.cpp
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_SOUNDCLIP_CPP_
#define FRAMEWORK_AUDIO_SOUNDCLIP_CPP_

#include <framework/audio/AudioManager.h>
#include <framework/audio/SoundClip.h>
#include <framework/audio/SoundPlayer.h>
#include <framework/audio/WaveFileReader.h>
#include <framework/io/files/FileInput.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/utility.h>

namespace rhfw {
namespace audio {

SoundClip::~SoundClip() {
	delete this->descriptor;
}

bool SoundClip::load() {
	if (this->descriptor == nullptr) {
		return false;
	}
	ASSERT(this->descriptor != nullptr) << "Input was not set";

	if (getDuration() < manager->getStreamDataMinDuration() * 2) {
		//load total instead of streaming
		totalData = this->descriptor->getTotalData();
	}

	return true;
}

void SoundClip::free() {
	for (auto& plr : dependentPlayers.objects()) {
		plr.stop();
	}
}

void SoundClip::setDescriptor(AudioDescriptor* descriptor) {
	delete this->descriptor;
	this->descriptor = descriptor;
}

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_SOUNDCLIP_CPP_ */
