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
 * OpenSLESPlayer.h
 *
 *  Created on: 2016. apr. 4.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_OPENSLES_ANDROID_OPENSLESPLAYER_H_
#define ANDROIDPLATFORM_OPENSLES_ANDROID_OPENSLESPLAYER_H_

#include <opensles_android/OpenSLESObjects.h>
#include <framework/audio/SoundClip.h>
#include <framework/audio/SoundPlayer.h>
#include <framework/audio/StreamData.h>
#include <framework/core/timing.h>
#include <framework/utils/LinkedList.h>
#include <gen/types.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace rhfw {
namespace audio {

class StreamHelper;
class OpenSLESAudioManager;
class StreamData;

class OpenSLESPlayer: public SoundPlayer, private core::TimeListener {
private:
	static void callback_headatend(SLPlayItf caller, void *pContext, SLuint32 event);
	static void callback_buffercompleted(SLAndroidSimpleBufferQueueItf caller, void *pContext);

	OpenSLESObject playerObject;
	OpenSLESInterface<SLPlayItf> playerItf;
	OpenSLESInterface<SLAndroidSimpleBufferQueueItf> bufferItf;

	core::time_millis playEndTime;
	bool headAtEnd = true;

	StreamHelper* streamHelper = nullptr;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	bool queueNextBuffers();
protected:
	virtual bool setVolumeGainImpl(float gain) override;
public:
	OpenSLESPlayer(AudioManager& manager, const SoundFormat& sformat, OpenSLESObject playerobject);
	~OpenSLESPlayer();

	OpenSLESPlayer* get() override {
		return this;
	}

	virtual bool play(SoundClip& clip) override;
	virtual void stop() override;

	PlayState getPlayState();
};
inline bool operator==(const SLDataFormat_PCM& a, const SLDataFormat_PCM& b) {
	return a.bitsPerSample == b.bitsPerSample && a.samplesPerSec == b.samplesPerSec && a.numChannels == b.numChannels
			&& a.channelMask == b.channelMask && a.containerSize == b.containerSize && a.endianness == b.endianness
			&& a.formatType == b.formatType;
}
inline bool operator!=(const SLDataFormat_PCM& a, const SLDataFormat_PCM& b) {
	return a.bitsPerSample != b.bitsPerSample || a.samplesPerSec != b.samplesPerSec || a.numChannels != b.numChannels
			|| a.channelMask != b.channelMask || a.containerSize != b.containerSize || a.endianness != b.endianness
			|| a.formatType != b.formatType;
}

} // namespace audio
} // namespace rhfw

#endif /* ANDROIDPLATFORM_OPENSLES_ANDROID_OPENSLESPLAYER_H_ */
