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
 * SoundStreamer.h
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_SOUNDSTREAMER_H_
#define FRAMEWORK_AUDIO_SOUNDSTREAMER_H_

#include <framework/core/timing.h>

namespace rhfw {
namespace audio {

class SoundStreamer {
private:
public:
	virtual ~SoundStreamer() = default;

	virtual StreamData getPcmData(long long pcmcount) = 0;
	virtual StreamData getData(core::time_millis duration) = 0;
	virtual long long getRemainingPcm() const = 0;
	virtual core::time_millis getRemainingDuration() const = 0;

	virtual void seek(long long pcmcount) = 0;
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_SOUNDSTREAMER_H_ */
