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
 * AudioDescriptor.h
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_AUDIODESCRIPTOR_H_
#define FRAMEWORK_AUDIO_AUDIODESCRIPTOR_H_

#include <framework/core/timing.h>
#include <framework/audio/StreamData.h>
#include <framework/audio/SoundStreamer.h>

#include <framework/utils/utility.h>

namespace rhfw {
namespace audio {
class SoundFormat;
class AudioDescriptor {
protected:
public:
	virtual ~AudioDescriptor() = default;

	virtual core::time_millis getDuration() = 0;

	virtual SoundStreamer* createStreamer() = 0;
	/**
	 * Returns the total audio data for the descriptor.
	 * Users are only allowed to query this once.
	 * If it was queried, then the streamer cannot be used for this descriptor.
	 */
	virtual StreamData getTotalData() {
		SoundStreamer* streamer = createStreamer();
		StreamData res = streamer->getPcmData(streamer->getRemainingPcm());
		delete streamer;
		return util::move(res);
	}

	virtual const SoundFormat& getFormat() const = 0;
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_AUDIODESCRIPTOR_H_ */
