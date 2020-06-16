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
//  OpenALSoundPlayer.cpp
//  TestApp
//
//  Created by User on 2016. 04. 10..
//  Copyright © 2016. User. All rights reserved.
//

#include <openal/OpenALSoundPlayer.h>
#include <openal/OpenALAudioManager.h>

namespace rhfw {
namespace audio {
#define ALMANAGER (static_cast<OpenALAudioManager&>(manager))

static ALenum convertFormat(const SoundFormat& format) {
	if (format.channels == 1) {
		if (format.bitsPerSample == 8) {
			return AL_FORMAT_MONO8;
		}
		ASSERT(format.bitsPerSample == 16) << "Invalid bits per sample: " << format.bitsPerSample;
		return AL_FORMAT_MONO16;
	} else {
		ASSERT(format.channels == 2) << "Invalid number of channels %" << format.channels;
		if (format.bitsPerSample == 8) {
			return AL_FORMAT_STEREO8;
		}
		ASSERT(format.bitsPerSample == 16) << "Invalid bits per sample: " << format.bitsPerSample;
		return AL_FORMAT_STEREO16;
	}
}

OpenALSoundPlayer::OpenALSoundPlayer(OpenALAudioManager& manager, const SoundFormat& format, ALuint source)
		: SoundPlayer(manager, format), source { source }, openAlFormat { convertFormat(format) } {
	ALMANAGER.alGenBuffers(1, buffers);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);
	generatedBuffers = 1;
	unqueuedBuffers[0] = buffers[0];
}
OpenALSoundPlayer::~OpenALSoundPlayer() {
	//do we have to unqueue? guess not
	ALMANAGER.alDeleteSources(1, &source);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);
	ALMANAGER.alDeleteBuffers(generatedBuffers, buffers);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);

	delete streamHelper;
	streamHelper = nullptr;
}

void OpenALSoundPlayer::onTimeChanged(const core::time_millis& time, const core::time_millis& prev) {
	if (this->getCurrentClip() != nullptr) {
		if (time >= playEndTime && (streamHelper == nullptr || (streamHelper->isAllBuffersSignaled() && !streamHelper->hasMoreData()))) {
			ALint value;
			ALMANAGER.alGetSourcei(source, AL_SOURCE_STATE, &value);
			CHECK_AL_ERROR_MANAGER(ALMANAGER);
			if (value == AL_STOPPED) {
				delete streamHelper;
				streamHelper = nullptr;

				if (queuedBufferCount > 0) {
					ALMANAGER.alSourceUnqueueBuffers(source, queuedBufferCount, queuedBuffers);
					CHECK_AL_ERROR_MANAGER(ALMANAGER);
					do {
						unqueuedBuffers[generatedBuffers - queuedBufferCount] = queuedBuffers[queuedBufferCount - 1];
						--queuedBufferCount;
					} while (queuedBufferCount > 0);
				}

				TimeListener::unsubscribe();
				onPlayerStopped();
			}
		} else if (streamHelper != nullptr) {
			ALint value;
			ALMANAGER.alGetSourcei(source, AL_BUFFERS_PROCESSED, &value);
			CHECK_AL_ERROR_MANAGER(ALMANAGER);
			if (value > 0) {
				ASSERT(value <= queuedBufferCount);
				ALMANAGER.alSourceUnqueueBuffers(source, value, queuedBuffers);
				CHECK_AL_ERROR_MANAGER(ALMANAGER);
				for (int i = 0; i < value; ++i) {
					unqueuedBuffers[generatedBuffers - queuedBufferCount] = queuedBuffers[i];
					--queuedBufferCount;
					streamHelper->signalBuffer();
				}
				//move buffers forward in array
				for (int i = 0; i < queuedBufferCount; ++i) {
					queuedBuffers[i] = queuedBuffers[value + i];
				}

				streamHelper->update();

				queueNextBuffers();

			}
		}

	}
}

void OpenALSoundPlayer::queueNextBuffers() {
	unsigned int startqueue = queuedBufferCount;
	while (queuedBufferCount < generatedBuffers) {
		auto* data = streamHelper->nextData();
		if (data == nullptr) {
			break;
		}

		queuedBuffers[queuedBufferCount] = unqueuedBuffers[generatedBuffers - queuedBufferCount - 1];
		ALMANAGER.alBufferData(queuedBuffers[queuedBufferCount], openAlFormat, data->getData(), data->getDataLength(),
				getFormat().sampleRate);
		//we are not using alBufferDataStatic, since it's bugged
		//on iOS, the source keeps reading from the buffer after it's stopped, or the buffer is unqueued

		++queuedBufferCount;
		streamHelper->signalBuffer();
	}
	if (startqueue != queuedBufferCount) {
		//update, so the allocated data will be freed. It is already moved to al context space via alBufferData
		streamHelper->update();
		ALMANAGER.alSourceQueueBuffers(source, queuedBufferCount - startqueue, queuedBuffers + startqueue);
		CHECK_AL_ERROR_MANAGER(ALMANAGER);
	}
}

bool OpenALSoundPlayer::play(SoundClip& clip) {
	ASSERT(queuedBufferCount == 0) << "Buffers are already queued" << queuedBufferCount;

	auto* totaldata = clip.getTotalData();
	if (totaldata == nullptr) {
		if (generatedBuffers < StreamHelper::BUFFERS_COUNT) {
			//generate remaining buffers
			ALMANAGER.alGenBuffers(StreamHelper::BUFFERS_COUNT - generatedBuffers, buffers + generatedBuffers);
			CHECK_AL_ERROR_MANAGER(ALMANAGER);
			for (int i = generatedBuffers; i < StreamHelper::BUFFERS_COUNT; ++i) {
				unqueuedBuffers[i] = buffers[i];
			}
			generatedBuffers = StreamHelper::BUFFERS_COUNT;
		}
		//streaming instead
		streamHelper = new StreamHelper { clip.createStreamer(), (long long) ((long long) manager.getStreamDataMinDuration()
				* getFormat().sampleRate / 1000.0 + 0.5) };

		queueNextBuffers();
	} else {
		queuedBufferCount = 1;
		queuedBuffers[0] = unqueuedBuffers[generatedBuffers - queuedBufferCount];
		//not using alBufferDataStatic, see queueNextBuffers()
		ALMANAGER.alBufferData(queuedBuffers[0], openAlFormat, totaldata->getData(), totaldata->getDataLength(), getFormat().sampleRate);
		//ALMANAGER.bufferDataStaticFunc(queuedBuffers[0], openAlFormat, totaldata->getData(), totaldata->getDataLength(), getFormat().sampleRate);
		ALMANAGER.alSourceQueueBuffers(source, queuedBufferCount, queuedBuffers);
		CHECK_AL_ERROR_MANAGER(ALMANAGER);
	}

	ALMANAGER.alSourcePlay(source);
	//if we exceed the maximum number of playing sources, alSourcePlay might return error (-1)
	if (ALenum __err = ALMANAGER.alGetError()) {
		ASSERT(__err == -1) << "Unknown error at alSourcePlay: " << __err;
		//unqueue the buffers
		ALMANAGER.alSourceUnqueueBuffers(source, queuedBufferCount, queuedBuffers);
		CHECK_AL_ERROR_MANAGER(ALMANAGER);

		while (queuedBufferCount > 0) {
			unqueuedBuffers[generatedBuffers - queuedBufferCount] = queuedBuffers[queuedBufferCount - 1];
			--queuedBufferCount;
		}
		delete streamHelper;
		streamHelper = nullptr;
		return false;
	}
	core::GlobalMonotonicTimeListener::subscribeListener(*this);
	playEndTime = core::MonotonicTime::getCurrent() + clip.getDuration();
	return true;
}

void OpenALSoundPlayer::stop() {
	ALMANAGER.alSourceStop(source);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);
	ALMANAGER.alSourceUnqueueBuffers(source, queuedBufferCount, queuedBuffers);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);

	while (queuedBufferCount > 0) {
		unqueuedBuffers[generatedBuffers - queuedBufferCount] = queuedBuffers[queuedBufferCount - 1];
		--queuedBufferCount;
	}

	delete streamHelper;
	streamHelper = nullptr;

	onPlayerStopped();
	TimeListener::unsubscribe();
}

bool OpenALSoundPlayer::setVolumeGainImpl(float gain) {
	ALMANAGER.alSourcef(source, AL_GAIN, gain);
	CHECK_AL_ERROR_MANAGER(ALMANAGER);
	return ALMANAGER.alGetError() == 0;
}

} // namespace rhfw
} // namespace audio

