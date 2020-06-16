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
 * OpenSLESPlayer.cpp
 *
 *  Created on: 2016. apr. 4.
 *      Author: sipka
 */

#include <opensles_android/OpenSLESPlayer.h>
#include <framework/audio/AudioManager.h>
#include <framework/audio/StreamHelper.h>
#include <framework/utils/BasicGlobalListener.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/utility.h>
#include <gen/log.h>

#include <math.h>

/**
 * On Samsung Note 3, when starting a new sound, the previous sound in the player could be heard at the start of the new.
 * Probably a bug, wasn't experienced on other Samsung devices.
 *
 */
namespace rhfw {
namespace audio {

void OpenSLESPlayer::callback_headatend(SLPlayItf caller, void *pContext, SLuint32 event) {
	OpenSLESPlayer* thiz = reinterpret_cast<OpenSLESPlayer*>(pContext);
	//calling any OpenSL function is undefined

	thiz->headAtEnd = true;
}

void OpenSLESPlayer::callback_buffercompleted(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
	OpenSLESPlayer* thiz = reinterpret_cast<OpenSLESPlayer*>(pContext);

	if (thiz->streamHelper != nullptr) {
		thiz->streamHelper->signalBuffer();
	}
}

OpenSLESPlayer::OpenSLESPlayer(AudioManager& manager, const SoundFormat& sformat, OpenSLESObject playerobject)
		: SoundPlayer { manager, sformat }, playerObject { util::move(playerobject) } {
	playerItf = playerObject;
	bufferItf = playerObject;
	ASSERT(playerItf != nullptr);
	ASSERT(bufferItf != nullptr);

	SLresult result;

	result = playerItf->SetCallbackEventsMask(playerItf, SL_PLAYEVENT_HEADATEND);
	CHECK_OPENSLES_ERROR();
	result = playerItf->RegisterCallback(playerItf, callback_headatend, this);
	CHECK_OPENSLES_ERROR();

	result = bufferItf->RegisterCallback(bufferItf, callback_buffercompleted, this);
	CHECK_OPENSLES_ERROR();

	//keep in playing state, and manage with Enqueue and Clear
	result = playerItf->SetPlayState(playerItf, SL_PLAYSTATE_PLAYING);
	CHECK_OPENSLES_ERROR();
}
OpenSLESPlayer::~OpenSLESPlayer() {
	//TimeListener automatically unsubscribes
	//clipNode removes itself from currentClip
	//sound is automatically stopped, if playing
	//we are removed from the Manager linked lists

	playerObject.destroy();

	delete streamHelper;
	streamHelper = nullptr;
}
bool OpenSLESPlayer::setVolumeGainImpl(float gain) {
	OpenSLESInterface<SLVolumeItf> volumeItf = playerObject;
	if (volumeItf == nullptr) {
		LOGW()<< "Failed to get SLVolumeItf from player";
		return false;
	}
	float db = 20.0f * log10(gain);
	float mb = db * 100;
	SLresult result = volumeItf->SetVolumeLevel(volumeItf, mb);
	CHECK_OPENSLES_ERROR();
	return result == SL_RESULT_SUCCESS;
}

void OpenSLESPlayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	if (this->getCurrentClip() != nullptr) {
		if (headAtEnd && time >= playEndTime
				&& (streamHelper == nullptr || (streamHelper->isAllBuffersSignaled() && !streamHelper->hasMoreData()))) {

			delete streamHelper;
			streamHelper = nullptr;

			TimeListener::unsubscribe();
			onPlayerStopped();

		} else if (streamHelper != nullptr) {
			streamHelper->update();

			queueNextBuffers();
		}
	}
}

bool OpenSLESPlayer::queueNextBuffers() {
	while (true) {
		auto* data = streamHelper->nextData();
		if (data == nullptr) {
			break;
		}
		SLresult result;
		result = bufferItf->Enqueue(bufferItf, data->getData(), data->getDataLength());
		CHECK_OPENSLES_ERROR();
		if (result != SL_RESULT_SUCCESS) {
			return false;
		}
	}
	return true;
}

bool OpenSLESPlayer::play(SoundClip& clip) {
	core::GlobalMonotonicTimeListener::subscribeListener(*this);

	playEndTime = core::MonotonicTime::getCurrent() + clip.getDuration();
	headAtEnd = false;

	SLresult result;
	auto* totaldata = clip.getTotalData();
	if (totaldata == nullptr) {
		//streaming
		streamHelper = new StreamHelper { clip.createStreamer(), (long long) ((long long) manager.getStreamDataMinDuration()
				* getFormat().sampleRate / 1000.0 + 0.5) };

		if (!queueNextBuffers()) {
			return false;
		}
	} else {
		//starts playing automatically
		result = bufferItf->Enqueue(bufferItf, totaldata->getData(), totaldata->getDataLength());
		CHECK_OPENSLES_ERROR();
		if (result != SL_RESULT_SUCCESS) {
			return false;
		}
	}
	return true;
}
void OpenSLESPlayer::stop() {
	SLresult result;
	result = bufferItf->Clear(bufferItf);
	CHECK_OPENSLES_ERROR();

	headAtEnd = true;

	delete streamHelper;
	streamHelper = nullptr;

	TimeListener::unsubscribe();
	onPlayerStopped();
}

PlayState OpenSLESPlayer::getPlayState() {
	SLresult result;
	SLuint32 playstate;

	result = playerItf->GetPlayState(playerItf, &playstate);
	CHECK_OPENSLES_ERROR();

	switch (playstate) {
		case SL_PLAYSTATE_STOPPED: {
			return PlayState::STOPPED;
		}
		case SL_PLAYSTATE_PAUSED: {
			return PlayState::PAUSED;
		}
		case SL_PLAYSTATE_PLAYING: {
			return PlayState::PLAYING;
		}
		default:
			THROW()<<"Invalid SL playstate value " << playstate;
			return PlayState::STOPPED;
		}
	}

}
// namespace audio
}// namespace rhfw

