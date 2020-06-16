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
 * SoundFormat.h
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_SOUNDFORMAT_H_
#define FRAMEWORK_AUDIO_SOUNDFORMAT_H_

#include <gen/types.h>

namespace rhfw {
namespace audio {

class SoundFormat {
private:
public:
	//count of the channels
	unsigned short channels;
	//sampling frequency in Hz
	unsigned int sampleRate;
	//size of a sample block
	unsigned short blockAlign;
	//valid bits per sample
	unsigned short bitsPerSample;
	//the endianness of the data
	Endianness endian;

	bool operator==(const SoundFormat& o) const {
		return channels == o.channels && sampleRate == o.sampleRate && blockAlign == o.blockAlign && bitsPerSample == o.bitsPerSample
				&& endian == o.endian;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_SOUNDFORMAT_H_ */
