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
#include <xaudio2/XAudio2SoundPlayer.h>
#include <xaudio2/XAudio2AudioManager.h>
#include WINDOWS_EXECPTION_HELPER_INCLUDE
#include <gen/log.h>

namespace rhfw {
namespace audio {

XAudio2SoundPlayer::XAudio2SoundPlayer(XAudio2AudioManager& manager, const SoundFormat& sformat, const PCMWAVEFORMAT& format)
		: SoundPlayer { manager, sformat }, sourceVoice { nullptr } {
	ThrowIfFailed(
			manager.getXAudio2()->CreateSourceVoice(&sourceVoice, (const tWAVEFORMATEX* )&format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, this));
}
XAudio2SoundPlayer::~XAudio2SoundPlayer() {
	if (sourceVoice != nullptr) {
		sourceVoice->DestroyVoice();

		//dont wait for itself to delete
		delete streamHelper;
	}
}

bool XAudio2SoundPlayer::queueNextBuffers() {
	while (true) {
		auto* data = streamHelper->nextData();
		if (data == nullptr) {
			break;
		}

		XAUDIO2_BUFFER buffer;
		buffer.pAudioData = (const BYTE*) data->getData();
		buffer.Flags = streamHelper->hasMoreData() ? 0 : XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = data->getDataLength();
		buffer.PlayBegin = 0;
		buffer.PlayLength = 0;
		buffer.LoopBegin = 0;
		buffer.LoopLength = 0;
		buffer.LoopCount = 0;
		buffer.pContext = streamHelper;

		HRESULT res = sourceVoice->SubmitSourceBuffer(&buffer);
		ASSERT(SUCCEEDED(res)) << res;
		if (!SUCCEEDED(res)) {
			return false;
		}
	}
	return true;
}

void XAudio2SoundPlayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (this->getCurrentClip() != nullptr) {
		if (streamEnded) {
			if (streamHelper != nullptr) {
				streamHelper->dispose();
				streamHelper = nullptr;
			}
			ThrowIfFailed(sourceVoice->FlushSourceBuffers());

			TimeListener::unsubscribe();
			onPlayerStopped();
		} else if (streamHelper != nullptr) {
			streamHelper->update();

			queueNextBuffers();
		}
	}
}

void XAudio2SoundPlayer::OnStreamEnd() {
	streamEnded = true;
}
void XAudio2SoundPlayer::OnBufferEnd(void* pBufferContext) {
	if (pBufferContext != nullptr) {
		static_cast<SelfDeleterStreamHelper*>(pBufferContext)->signalBuffer();
	}
}

bool XAudio2SoundPlayer::play(SoundClip& clip) {
	ThrowIfFailed(sourceVoice->Stop());
	ThrowIfFailed(sourceVoice->FlushSourceBuffers());

	core::GlobalMonotonicTimeListener::subscribeListener(*this);
	streamEnded = false;
	auto currenttime = core::MonotonicTime::getCurrent();

	auto* totaldata = clip.getTotalData();
	if (totaldata == nullptr) {
		//streaming	
		streamHelper = new SelfDeleterStreamHelper { clip.createStreamer(), (long long) ((long long) manager.getStreamDataMinDuration()
				* getFormat().sampleRate / 1000.0 + 0.5) };
		core::GlobalMonotonicTimeListener::subscribeListener(*streamHelper);

		if (!queueNextBuffers()) {
			return false;
		}
	} else {
		//queue 1 buffer with total data
		XAUDIO2_BUFFER buffer;
		buffer.pAudioData = (const BYTE*) totaldata->getData();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = totaldata->getDataLength();
		buffer.PlayBegin = 0;
		buffer.PlayLength = 0;
		buffer.LoopBegin = 0;
		buffer.LoopLength = 0;
		buffer.LoopCount = 0;
		buffer.pContext = nullptr;

		HRESULT res = sourceVoice->SubmitSourceBuffer(&buffer);
		ASSERT(SUCCEEDED(res)) << res;
		if (!SUCCEEDED(res)) {
			return false;
		}
	}
	HRESULT res = sourceVoice->Start(0);
	ASSERT(SUCCEEDED(res)) << res;
	if (!SUCCEEDED(res)) {
		ThrowIfFailed(sourceVoice->FlushSourceBuffers());
		return false;
	}
	return true;
}

void XAudio2SoundPlayer::stop() {
	streamEnded = true;
	if (streamHelper != nullptr) {
		streamHelper->dispose();
		streamHelper = nullptr;
	}
	ThrowIfFailed(sourceVoice->Stop());
	ThrowIfFailed(sourceVoice->FlushSourceBuffers());

	TimeListener::unsubscribe();
	onPlayerStopped();
}

void XAudio2SoundPlayer::SelfDeleterStreamHelper::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (disposed && isAllBuffersSignaled()) {
		delete this;
	}
}

bool XAudio2SoundPlayer::setVolumeGainImpl(float gain) {
	HRESULT res = sourceVoice->SetVolume(gain, XAUDIO2_COMMIT_NOW);
	ASSERT(SUCCEEDED(res)) << res;
	return SUCCEEDED(res);
}
} // namespace audio
} // namespace rhfw

