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
//  OpenALAudioManager.h
//  TestApp
//
//  Created by User on 2016. 04. 10..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef OpenALAudioManager_h
#define OpenALAudioManager_h

#include <framework/audio/AudioManager.h>
#include <gen/configuration.h>
#include <openalglue/openalglue.h>

#if RHFW_DEBUG
#define CHECK_ALC_ERROR() if(ALCenum __err = alcGetError(device)) { THROW() << "ALC has error state: " << __err; }
#define CHECK_AL_ERROR() if(ALenum __err = alGetError()) { THROW() << "OpenAL has error state: " << __err; }
#define CHECK_AL_ERROR_MANAGER(man) if(ALenum __err = (man).alGetError()) { THROW() << "OpenAL has error state: " << __err; }
#else
#define CHECK_ALC_ERROR()
#define CHECK_AL_ERROR()
#define CHECK_AL_ERROR_MANAGER(man)
#endif

namespace rhfw {
namespace audio {

class OpenALAudioManager: public AudioManager, public OpenAlGlue {
private:
	friend class OpenALSoundPlayer;

	ALCdevice* device = nullptr;
	ALCcontext* context = nullptr;

	virtual SoundPlayer* createSoundPlayerImpl(const SoundFormat& format) override;

	virtual bool loadImpl() override;
	virtual void freeImpl() override;
protected:
public:
	OpenALAudioManager();
	~OpenALAudioManager();

	virtual void playMultiple(SoundClip* clips) override;
};

} // namespace rhfw
} // namespace audio

#endif /* OpenALAudioManager_h */
