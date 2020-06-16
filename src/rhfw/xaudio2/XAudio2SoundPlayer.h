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
#ifndef XAUDIO2SOUNDPLAYER_H_
#define XAUDIO2SOUNDPLAYER_H_

#include <framework/audio/SoundPlayer.h>
#include <framework/audio/StreamHelper.h>
#include <framework/utils/LinkedList.h>
#include <framework/core/timing.h>

#include <xaudio2/xaudio2_header.h>
#include <gen/log.h>

namespace rhfw {
namespace audio {
class StreamData;
class XAudio2AudioManager;

class XAudio2SoundPlayer: public SoundPlayer, private IXAudio2VoiceCallback, private core::TimeListener {
	friend class XAudio2AudioManager;
private:
	IXAudio2SourceVoice* sourceVoice;
	bool streamEnded = true;

	class SelfDeleterStreamHelper: public StreamHelper, public core::TimeListener {
		virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;
		bool disposed = false;
	public:
		using StreamHelper::StreamHelper;

		void dispose() {
			disposed = true;
		}
		bool isDisposed() const {
			return disposed;
		}
	};
	SelfDeleterStreamHelper* streamHelper = nullptr;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	// Called just before this voice's processing pass begins.
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) override {
	}

	// Called just after this voice's processing pass ends.
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) override {
	}

	// Called when this voice has just finished playing a buffer stream
	// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
	STDMETHOD_(void, OnStreamEnd) (THIS) override;

	// Called when this voice is about to start processing a new buffer.
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) override {
	}

	// Called when this voice has just finished processing a buffer.
	// The buffer can now be reused or destroyed.
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) override;

	// Called when this voice has just reached the end position of a loop.
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) override {
	}

	// Called in the event of a critical error during voice processing,
	// such as a failing xAPO or an error from the hardware XMA decoder.
	// The voice may have to be destroyed and re-created to recover from
	// the error.  The callback arguments report which buffer was being
	// processed when the error occurred, and its HRESULT code.
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) override {
		THROW() << "Not implemented";
	}

	bool queueNextBuffers();
protected:
	virtual bool setVolumeGainImpl(float gain) override;
public:
	XAudio2SoundPlayer(XAudio2AudioManager& manager, const SoundFormat& sformat, const PCMWAVEFORMAT& format);
	~XAudio2SoundPlayer();
	virtual bool play(SoundClip& clip) override;
	virtual void stop() override;

};

} // namespace audio
} // namespace rhfw

#endif /* XAUDIO2SOUNDPLAYER_H_ */
