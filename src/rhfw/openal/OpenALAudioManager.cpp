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
//
//  OpenALAudioManager.cpp
//  TestApp
//
//  Created by User on 2016. 04. 10..
//  Copyright © 2016. User. All rights reserved.
//

#include <openal/OpenALAudioManager.h>
#include <openal/OpenALSoundPlayer.h>

#include <gen/audiomanagers.h>

namespace rhfw {
namespace audio {

SoundPlayer* OpenALAudioManager::createSoundPlayerImpl(const SoundFormat& format) {
	ALuint src;
	alGenSources(1, &src);
	CHECK_AL_ERROR();

	return new OpenALSoundPlayer { *this, format, src };
}

bool OpenALAudioManager::loadImpl() {
	if (!OpenAlGlue::load()) {
		return false;
	}
	device = alcOpenDevice(nullptr);
	CHECK_ALC_ERROR();
	ASSERT(device != nullptr) << "Failed to open device";

	context = alcCreateContext(device, nullptr);
	CHECK_ALC_ERROR();
	ASSERT(context != nullptr) << "Failed to open context";

	alcMakeContextCurrent(context);
	CHECK_ALC_ERROR();

	return true;
}

void OpenALAudioManager::freeImpl() {
	alcMakeContextCurrent(nullptr);
	CHECK_ALC_ERROR();
	alcDestroyContext(context);
	CHECK_ALC_ERROR();
	//alcCloseDevice doesn't have return value in 1.0
	alcCloseDevice(device);
	//on macOS even if alcCloseDevice returns ALC_TRUE, the next getError fails
	//CHECK_ALC_ERROR();

	OpenAlGlue::free();
}

OpenALAudioManager::OpenALAudioManager() {
}

OpenALAudioManager::~OpenALAudioManager() {
}

void OpenALAudioManager::playMultiple(SoundClip* clips) {

}

template<> audio::AudioManager* instantiateAudioManager<AudioConfig::OpenAL10>() {
	return new OpenALAudioManager();
}

} // namespace rhfw
} // namespace audio
