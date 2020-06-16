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
 * WaveFileReader.cpp
 *
 *  Created on: 2016. apr. 3.
 *      Author: sipka
 */

#include <framework/audio/WaveFileReader.h>
#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/BufferedInputStream.h>

#include <gen/log.h>

namespace rhfw {
namespace audio {

#define TAG_RIFF 0x46464952
#define TAG_WAVE 0x45564157
#define TAG_fmt_ 0x20746d66
#define TAG_data 0x61746164

template<typename Stream>
bool WaveFileReader::handleSubChunk(Stream& stream) {
	uint32 tag;
	if (stream.template deserialize<Endianness::Little, uint32>(tag)) {
		switch (tag) {
			case TAG_fmt_: {
				unsigned int chunksize;
				stream.template deserialize<Endianness::Little, uint32>(chunksize);
				ASSERT(chunksize >= 16) << "Not PCM data " << chunksize;

				unsigned short audioformat;
				stream.template deserialize<Endianness::Little, uint16>(audioformat);
				ASSERT(audioformat == 1) << "Not PCM format, " << (unsigned int) audioformat;

				uint32 byterate;

				stream.template deserialize<Endianness::Little, uint16>(format.channels);
				stream.template deserialize<Endianness::Little, uint32>(format.sampleRate);
				stream.template deserialize<Endianness::Little, uint32>(byterate);
				stream.template deserialize<Endianness::Little, uint16>(format.blockAlign);
				stream.template deserialize<Endianness::Little, uint16>(format.bitsPerSample);

				if (chunksize > 16) {
					stream.seek(chunksize - 16, SeekMethod::CURRENT);
				}
				break;
			}
			case TAG_data: {
				stream.template deserialize<Endianness::Little, uint32>(format.dataLength);
				format.fileOffset = stream.getPosition();
				stream.seek(format.dataLength, SeekMethod::CURRENT);
				break;
			}
			default: {
				uint32 chunksize;
				if (stream.template deserialize<Endianness::Little, uint32>(chunksize)) {
					stream.seek(chunksize, SeekMethod::CURRENT);
					LOGV()<< "Unknown subchunk tag: " << tag;
				} else {
					return false;
				}
				break;
			}
		}
		return true;
	}
	return false;
}
audio::WaveFileReader::WaveFileReader(FileInput& input) {
	format.dataLength = 0;
	format.fileOffset = 0;
	format.endian = Endianness::Little;
	input.open();

	auto stream = BufferedInputStream::wrap(input);

	uint32 RIFF;
	uint32 riffchunksize;
	uint32 fileformat;

	stream.deserialize<Endianness::Little, uint32>(RIFF);
	stream.deserialize<Endianness::Little, uint32>(riffchunksize);
	stream.deserialize<Endianness::Little, uint32>(fileformat);

	ASSERT(RIFF == TAG_RIFF) << "RIFF missing from wave file " << RIFF;
	ASSERT(fileformat == TAG_WAVE) << "WAVE missing from RIFF in wave file " << fileformat;

	//two subchunks, "data" and "fmt "
	while (handleSubChunk(stream)) {
		//just execute the condition
	}
}

audio::WaveFileReader::WaveFileReader(FileInput* input)
		: WaveFileReader(*input) {
	input->close();
	delete input;
}

audio::WaveFileReader::WaveFileReader(FileDescriptor& desc)
		: WaveFileReader(desc.createInput()) {
}

audio::WaveFileReader::WaveFileReader(FileInput&& input)
		: WaveFileReader(input) {
}

audio::WaveFileReader::WaveFileReader(FileDescriptor&& desc)
		: WaveFileReader(desc) {
}

}  // namespace audio
}  // namespace rhfw

