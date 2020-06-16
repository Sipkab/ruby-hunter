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
#ifndef OGGVORBISAUDIODESCRIPTOR_H_
#define OGGVORBISAUDIODESCRIPTOR_H_

#include <framework/audio/AudioDescriptor.h>
#include <framework/audio/StreamData.h>
#include <framework/core/timing.h>
#include <framework/audio/SoundStreamer.h>
#include <framework/io/files/FileInput.h>
#include <framework/audio/WaveFileReader.h>
#include <framework/io/files/FileDescriptor.h>

#include <gen/log.h>

#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

namespace rhfw {
namespace audio {

class OggVorbisAudioDescriptor: public AudioDescriptor {
private:
	static void streamDataReleaser(StreamData& sd, void* param) {
		//delete allocated storage
		delete[] sd.getData();
	}

	static size_t read_callback_func(void *ptr, size_t size, size_t nmemb, void *datasource) {
		auto* input = static_cast<FileInput*>(datasource);
		int res = input->read(ptr, size * nmemb);
		if (res < 0) {
			return 0;
		}
		return res;
		/*unsigned int out;
		 auto* res = input->read(size * nmemb, &out);
		 for (unsigned int i = 0; i < out; i++) {
		 static_cast<char*>(ptr)[i] = res[i];
		 }
		 return out;*/
	}
	static int seek_callback_func(void *datasource, ogg_int64_t offset, int whence) {
		auto* input = static_cast<FileInput*>(datasource);
		SeekMethod method;
		switch (whence) {
			case SEEK_CUR:
				method = SeekMethod::CURRENT;
				break;
			case SEEK_SET:
				method = SeekMethod::BEGIN;
				break;
			case SEEK_END:
				method = SeekMethod::END;
				break;
			default: {
				THROW() << "Unknown seek method: " << whence;
				return false;
			}
		}
		return (int) input->seek(offset, method);
	}
	static int close_callback_func(void *datasource) {
		auto* input = static_cast<FileInput*>(datasource);
		input->close();
		delete input;
		return 0;
	}
	static long tell_callback_func(void *datasource) {
		auto* input = static_cast<FileInput*>(datasource);
		return (long) input->getPosition();
	}

	class OggVorbisAudioStreamer: public SoundStreamer {
	private:
		static const unsigned int BUFFER_SIZE = 4096;
		OggVorbisAudioDescriptor& descriptor;
		FileInput* input;

		char buffer[BUFFER_SIZE];
		int bufferCount = 0;
		int bufferOffset = 0;

		long long remainingPcm;

		OggVorbis_File vf;
	public:

		OggVorbisAudioStreamer(OggVorbisAudioDescriptor& descriptor)
				: descriptor(descriptor), input { descriptor.fileDescriptor->createInput() } {
			input->open();
			int res = ov_open_callbacks(input, &vf, nullptr, 0, ov_callbacks { read_callback_func, seek_callback_func, close_callback_func,
					tell_callback_func });
			if (res != 0) {
				remainingPcm = 0;
			}
			ASSERT(res == 0) << "Failed to open ogg vorbis file: " << res;
			remainingPcm = (long long) ov_pcm_total(&vf, -1);
		}
		~OggVorbisAudioStreamer() {
			ov_clear(&vf);
		}
		virtual StreamData getPcmData(long long pcmcount) override {
			if (pcmcount == 0)
				return nullptr;

			auto& format = descriptor.format;
			unsigned int byteslength = pcmcount * format.blockAlign;

			char* data = new char[byteslength];
			unsigned int dataoffset = 0;
			while (dataoffset < byteslength) {
				if (bufferCount > 0) {
					data[dataoffset++] = buffer[bufferOffset];
					--bufferCount;
					++bufferOffset;
				} else {
					bufferOffset = 0;

					int bitstream;
					//nothing in the buffer
					bufferCount = ov_read(&vf, buffer, BUFFER_SIZE, 0, format.blockAlign, 1, &bitstream);
					if (bufferCount <= 0)
						break;
				}
			}
			if (dataoffset == 0) {
				delete[] data;
				return nullptr;
			}

			remainingPcm -= dataoffset / format.blockAlign;

			return StreamData { streamDataReleaser, data, dataoffset, data };
		}
		virtual StreamData getData(core::time_millis duration) override {
			auto& format = descriptor.format;
			return getPcmData((long long) ((long long) duration * format.sampleRate / 1000.0 + 0.5));
		}
		virtual long long getRemainingPcm() const override {
			return remainingPcm;
		}
		virtual core::time_millis getRemainingDuration() const override {
			return core::time_millis { (long long) getRemainingPcm() * 1000 / (descriptor.format.sampleRate * descriptor.format.blockAlign) };
		}

		virtual void seek(long long pcmcount) override {
			int res = ov_pcm_seek(&vf, pcmcount);
			ASSERT(res == 0) << "Failed to seek ogg vorbis stream";
		}
	};

	SoundFormat format;
	FileDescriptor* fileDescriptor;

	core::time_millis totalDuration;

	OggVorbisAudioDescriptor(FileDescriptor* desc, OggVorbis_File& vf)
			: fileDescriptor(desc) {
		vorbis_info* info = ov_info(&vf, -1);
		format.channels = info->channels;
		format.sampleRate = info->rate;
		format.blockAlign = 2;
		format.bitsPerSample = 16;
		format.endian = Endianness::Little;

		totalDuration = core::time_millis { (long long) (ov_time_total(&vf, -1) * 1000.0) };

		ov_clear(&vf);
	}

public:
	static OggVorbisAudioDescriptor* create(FileDescriptor* desc) {
		OggVorbis_File vf;
		FileInput* input = desc->createInput();
		input->open();
		//we have to open, else ov_time_total returns error, it has to be opened.
		int res = ov_open_callbacks(input, &vf, nullptr, 0, ov_callbacks { read_callback_func, seek_callback_func, close_callback_func,
				tell_callback_func });
		if (res != 0 || vf.links != 1) {
			input->close();
			delete input;
			return nullptr;
		}
		return new OggVorbisAudioDescriptor(desc, vf);
	}

	~OggVorbisAudioDescriptor() {
		delete fileDescriptor;
	}

	virtual core::time_millis getDuration() override {
		return totalDuration;
	}

	virtual SoundStreamer* createStreamer() override {
		return new OggVorbisAudioStreamer { *this };
	}
	virtual StreamData getTotalData() override {
		OggVorbis_File vf;
		FileInput* input = fileDescriptor->createInput();
		input->open();
		int res = ov_open_callbacks(input, &vf, nullptr, 0, ov_callbacks { read_callback_func, seek_callback_func, close_callback_func,
				tell_callback_func });
		if (res != 0) {
			return nullptr;
		}
		ASSERT(res == 0) << "Failed to open ogg vorbis file: " << res;

		//XXX be careful, Note 3 doesnt like very small samples with OpenSLES, (around 100ms) becomes laggy
		auto totalpcm = ov_pcm_total(&vf, -1);
		auto totalbytes = totalpcm * format.blockAlign;
		char* data = new char[totalbytes];

		int bitstream;
		int read;
		int offset = 0;
		while (true) {
			read = ov_read(&vf, data + offset, totalbytes - offset, 0, format.blockAlign, 1, &bitstream);
			if (read > 0) {
				offset += read;
			} else {
				break;
			}
		}

		ov_clear(&vf);

		if (offset <= 0) {
			delete[] data;
			return nullptr;
		}
		return StreamData { streamDataReleaser, data, (unsigned int) offset, data };
	}
	virtual const SoundFormat& getFormat() const override {
		return format;
	}
};

} // namespace audio
} // namespace rhfw

#endif /* OGGVORBISAUDIODESCRIPTOR_H_ */
