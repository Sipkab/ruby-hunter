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
 * WaveFileReader.h
 *
 *  Created on: 2016. apr. 3.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_WAVEFILEREADER_H_
#define FRAMEWORK_AUDIO_WAVEFILEREADER_H_

#include <framework/audio/SoundFormat.h>
#include <framework/core/timing.h>

namespace rhfw {
class FileDescriptor;
class FileInput;
namespace audio {

class WaveFileFormat: public SoundFormat {
private:
public:
	unsigned int dataLength;
	unsigned long long fileOffset;

	core::time_millis getDuration() const {
		return core::time_millis { (long long) (dataLength / (double) (sampleRate * blockAlign) * 1000.0) };
	}
};

class WaveFileReader {
public:
private:
	WaveFileFormat format;

	template<typename Stream>
	bool handleSubChunk(Stream& input);
	WaveFileReader(FileInput* input);
public:
	WaveFileReader(FileInput& input);
	WaveFileReader(FileDescriptor& desc);
	WaveFileReader(FileInput&& input);
	WaveFileReader(FileDescriptor&& desc);
	WaveFileReader(WaveFileReader&&) = default;
	WaveFileReader(const WaveFileReader&) = default;
	WaveFileReader& operator=(WaveFileReader&&) = default;
	WaveFileReader& operator=(const WaveFileReader&) = default;

	const WaveFileFormat& getFormat() const {
		return format;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_WAVEFILEREADER_H_ */
