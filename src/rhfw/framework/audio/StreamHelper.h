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
 * StreamHelper.h
 *
 *  Created on: 2016. maj. 6.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_STREAMHELPER_H_
#define FRAMEWORK_AUDIO_STREAMHELPER_H_

#include <framework/audio/SoundStreamer.h>
#include <framework/audio/StreamData.h>
#include <framework/core/timing.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LinkedNode.h>
#include <gen/log.h>

namespace rhfw {
namespace audio {

class StreamHelper {
public:
	static const unsigned int BUFFERS_COUNT = 3;
private:
	SoundStreamer* streamer;
	long long minPcmCount;

	StreamData datas[BUFFERS_COUNT];
	//this class represents and index, that can only be modified by one thread, and read by another
	template<unsigned int MAX>
	class thread_atomic_small_index {
		//we expect, that the writing of a byte is atomic, and no hazardous value will be read
		//on other platforms this could be larger than a byte, however an unsigned char suits our needs
		unsigned char index = 0;
	public:
		thread_atomic_small_index(unsigned char initial)
				: index { initial } {
		}
		operator unsigned char() const {
			return index;
		}
		thread_atomic_small_index<MAX>& operator++() {
			index = (index + 1) % MAX;
			return *this;
		}
	};
	thread_atomic_small_index<BUFFERS_COUNT> buffersProcessed = 0;
	thread_atomic_small_index<BUFFERS_COUNT> buffersSignaled = 0;
	thread_atomic_small_index<BUFFERS_COUNT> buffersStart = 0;
	unsigned int queuedBufferCount = 0;

public:
	StreamHelper(SoundStreamer* streamer, long long minpcmcount)
			: streamer { streamer }, minPcmCount { minpcmcount } {
		ASSERT(streamer != nullptr) << "Streamer is null";
	}
	StreamHelper(StreamHelper&& o) = delete;
	StreamHelper(const StreamHelper&) = delete;
	~StreamHelper() {
		delete streamer;
	}

	/**
	 * Can be called only from the dedicated audio processing thread, or the main thread
	 */
	void signalBuffer() {
		++buffersSignaled;
	}

	bool isAllBuffersSignaled() const {
		return buffersProcessed == buffersSignaled;
	}

	void update() {
		while (buffersProcessed != buffersSignaled) {
			datas[buffersProcessed % BUFFERS_COUNT] = nullptr;
			++buffersProcessed;
			--queuedBufferCount;
		}
	}

	bool hasMoreData() const {
		return streamer->getRemainingPcm() > 0;
	}

	StreamData* nextData() {
		if (queuedBufferCount == BUFFERS_COUNT) {
			return nullptr;
		}
		auto remaining = streamer->getRemainingPcm();
		if (remaining == 0) {
			return nullptr;
		}
		auto& d = datas[buffersStart % BUFFERS_COUNT];
		if (remaining < minPcmCount * 2) {
			//we cant split it to two
			d = streamer->getPcmData(remaining);
		} else if (remaining >= minPcmCount * 3) {
			//we still can return minduration
			d = streamer->getPcmData(minPcmCount);
		} else {
			d = streamer->getPcmData(remaining / 2);
		}
		if (d == nullptr) {
			return nullptr;
		}
		++queuedBufferCount;
		++buffersStart;
		return &d;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_STREAMHELPER_H_ */
