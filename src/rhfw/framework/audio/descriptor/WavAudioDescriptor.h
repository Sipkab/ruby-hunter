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
#ifndef WAVAUDIODESCRIPTOR_H_
#define WAVAUDIODESCRIPTOR_H_

#include <framework/audio/AudioDescriptor.h>
#include <framework/audio/StreamData.h>
#include <framework/core/timing.h>
#include <framework/audio/SoundStreamer.h>
#include <framework/io/files/FileInput.h>
#include <framework/audio/WaveFileReader.h>
#include <framework/io/files/FileDescriptor.h>

namespace rhfw {
namespace audio {

class WavAudioDescriptor: public AudioDescriptor {
private:
	static void totalDataReleaser(StreamData&, void* param) {
		//delete taken buffer data
		delete[] static_cast<char*>(param);
	}
	static void dataDeleterReleaser(StreamData& d, void*) {
		//delete taken buffer data
		delete[] d.getData();
	}
	class WavAudioStreamer: public SoundStreamer {
	private:
		WavAudioDescriptor& descriptor;
		long long offsetSeek = 0;
	public:
		WavAudioStreamer(WavAudioDescriptor& descriptor)
				: descriptor(descriptor) {
		}
		virtual StreamData getPcmData(long long pcmcount) override {
			auto& format = descriptor.format;
			long long seek = format.fileOffset + offsetSeek;
			unsigned int length = (unsigned int) (pcmcount * format.blockAlign);

			auto* input = descriptor.input;
			//we assume input is already open
			ASSERT(input->isOpened()) << "Audio input file is not opened";
			input->seek(seek, SeekMethod::BEGIN);

			char* resptr = new char[length];
			int res = input->read(resptr, length);
			if (res <= 0) {
				delete[] resptr;
				return nullptr;
			}
			offsetSeek += res;
			return StreamData { dataDeleterReleaser, resptr, (unsigned int) res };

			/*	unsigned int got;
			 const char* audioData = input->read(length, &got);
			 if (got == 0) {
			 return nullptr;
			 }

			 char* resptr = new char[got];
			 for (unsigned int i = 0; i < got; ++i) {
			 resptr[i] = audioData[i];
			 }

			 offsetSeek += got;

			 return StreamData { dataDeleterReleaser, resptr, got };*/
		}
		virtual StreamData getData(core::time_millis duration) override {
			auto& format = descriptor.format;
			return getPcmData((long long) ((long long) duration * format.sampleRate / 1000.0 + 0.5));
		}
		virtual long long getRemainingPcm() const override {
			return (descriptor.format.dataLength - offsetSeek) / descriptor.format.blockAlign;
		}
		virtual core::time_millis getRemainingDuration() const override {
			return descriptor.getDuration()
					- core::time_millis { (long long) (offsetSeek / (double) (descriptor.format.sampleRate * descriptor.format.blockAlign)
							* 1000.0) };
		}

		virtual void seek(long long pcmcount) override {
			offsetSeek = pcmcount * descriptor.format.blockAlign;
		}
	};

	WaveFileFormat format;
	FileDescriptor* fileDescriptor;
	FileInput* input;
public:
	WavAudioDescriptor(FileDescriptor* desc)
			: fileDescriptor(desc), input { desc->createInput() } {
		WaveFileReader reader(*input);
		this->format = reader.getFormat();
	}
	~WavAudioDescriptor() {
		input->close();
		delete input;
		delete fileDescriptor;
	}

	virtual core::time_millis getDuration() override {
		return format.getDuration();
	}

	virtual SoundStreamer* createStreamer() override {
		ASSERT(!input->isClosed());
		return new WavAudioStreamer { *this };
	}
	virtual StreamData getTotalData() override {
		//XXX be careful, Note 3 doesnt like very small samples with OpenSLES, (around 100ms) becomes laggy
		input->seek(format.fileOffset, SeekMethod::BEGIN);
		unsigned int audioSize = format.dataLength;
		char* data = new char[audioSize];
		int res = input->read(data, audioSize);
		if (res <= 0) {
			return nullptr;
		}
		return StreamData { dataDeleterReleaser, data, (unsigned int) res };
	}
	virtual const SoundFormat& getFormat() const override {
		return format;
	}
};

} // namespace audio
} // namespace rhfw

#endif /* WAVAUDIODESCRIPTOR_H_ */
