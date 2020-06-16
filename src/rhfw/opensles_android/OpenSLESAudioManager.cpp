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
 * OpenSLESAudioManager.cpp
 *
 *  Created on: 2016. marc. 4.
 *      Author: sipka
 */

#include <opensles_android/OpenSLESAudioManager.h>
#include <framework/io/files/AssetFileDescriptor.h>

#include <gen/log.h>
#include <gen/audiomanagers.h>

namespace rhfw {
namespace audio {

OpenSLESAudioManager::OpenSLESAudioManager() {
}

OpenSLESAudioManager::~OpenSLESAudioManager() {
}

bool OpenSLESAudioManager::loadImpl() {
	SLresult result;

	result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
	CHECK_OPENSLES_ERROR();

	result = engineObject.realize();
	CHECK_OPENSLES_ERROR();

	engineItf = engineObject;

	result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 0, nullptr, nullptr);
	CHECK_OPENSLES_ERROR();

	result = outputMixObject.realize();
	CHECK_OPENSLES_ERROR();
	return true;
}

void OpenSLESAudioManager::freeImpl() {
	outputMixObject.destroy();
	engineObject.destroy();
	engineItf = nullptr;
}

SoundPlayer* OpenSLESAudioManager::createSoundPlayerImpl(const SoundFormat& format) {
	SLDataLocator_AndroidSimpleBufferQueue inputlocation;
	SLDataSource source;
	SLDataLocator_OutputMix outputlocation;
	SLDataSink sink;
	SLDataFormat_PCM lformat;
	lformat.formatType = SL_DATAFORMAT_PCM;
	lformat.numChannels = format.channels;
	//OpenSL misnamed, samplesPerSec requires milliHertz
	lformat.samplesPerSec = format.sampleRate * 1000;
	lformat.bitsPerSample = format.bitsPerSample;
	lformat.containerSize = format.blockAlign * 8;
	lformat.channelMask = SL_SPEAKER_FRONT_CENTER;
	lformat.endianness = convertEndianness(format.endian);

	//in the android source code 4 buffers are always allocated, but we need to request at least this many to use them.
	//we request 4 buffer to be able to stream content
	inputlocation.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
	inputlocation.numBuffers = 4;

	outputlocation.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	outputlocation.outputMix = this->outputMixObject;

	source.pLocator = &inputlocation;
	source.pFormat = &lformat;

	sink.pLocator = &outputlocation;
	sink.pFormat = nullptr;

	OpenSLESObject player;

	const unsigned int ID_COUNT = 2;
	SLInterfaceID ids[ID_COUNT] { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME };
	SLboolean requireds[ID_COUNT] { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	SLresult result = this->engineItf->CreateAudioPlayer(this->engineItf, &player, &source, &sink, ID_COUNT, ids, requireds);
	if (result == SL_RESULT_SUCCESS) {
		result = player.realize();
		if (result != SL_RESULT_SUCCESS) {
			player.destroy();
			return nullptr;
		}
		return new OpenSLESPlayer(*this, format, util::move(player));
	} else if (result == SL_RESULT_MEMORY_FAILURE) {
		return nullptr;
	}
	CHECK_OPENSLES_ERROR();

	return nullptr;
}

void OpenSLESAudioManager::playMultiple(SoundClip* clips) {
	for (; clips != nullptr; ++clips) {
	}
}

template<> audio::AudioManager* instantiateAudioManager<AudioConfig::OpenSLES10Android>() {
	return new OpenSLESAudioManager { };
}

}  // namespace audio
}  // namespace rhfw

