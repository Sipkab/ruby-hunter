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
//  OpenALSoundPlayer.h
//  TestApp
//
//  Created by User on 2016. 04. 10..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef OpenALSoundPlayer_hpp
#define OpenALSoundPlayer_hpp

#include <framework/audio/SoundPlayer.h>
#include <framework/utils/LinkedList.h>
#include <framework/core/timing.h>
#include <framework/audio/StreamHelper.h>

#include <openal/OpenALAudioManager.h>

#include <gen/configuration.h>

namespace rhfw {
namespace audio {

class StreamHelper;
class OpenALAudioManager;

class OpenALSoundPlayer: public SoundPlayer, private core::TimeListener {
private:
	ALuint source;
	ALuint buffers[StreamHelper::BUFFERS_COUNT];
	ALsizei generatedBuffers = 0;

	ALuint queuedBuffers[StreamHelper::BUFFERS_COUNT];
	ALuint queuedBufferCount = 0;
	ALuint unqueuedBuffers[StreamHelper::BUFFERS_COUNT];

	ALenum openAlFormat;

	core::time_millis playEndTime;

	StreamHelper* streamHelper = nullptr;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& prev) override;

	void queueNextBuffers();
protected:
	virtual bool setVolumeGainImpl(float gain) override;
public:
	OpenALSoundPlayer(OpenALAudioManager& manager, const SoundFormat& format, ALuint source);
	~OpenALSoundPlayer();
	virtual bool play(SoundClip& clip) override;
	virtual void stop() override;
};

} // namespace rhfw
} // namespace audio

#endif /* OpenALSoundPlayer_hpp */
