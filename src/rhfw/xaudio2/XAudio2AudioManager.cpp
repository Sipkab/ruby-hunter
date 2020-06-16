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
#include <xaudio2/XAudio2AudioManager.h>
#include WINDOWS_EXECPTION_HELPER_INCLUDE
#include <xaudio2/XAudio2SoundPlayer.h>

#include <objbase.h>

#include <gen/log.h>
#include <gen/audiomanagers.h>
#include <gen/platform.h>

namespace rhfw {
namespace audio {

SoundPlayer* XAudio2AudioManager::createSoundPlayerImpl(const SoundFormat& format) {
	PCMWAVEFORMAT waveformat;

	waveformat.wf.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.wf.nChannels = format.channels;
	waveformat.wf.nSamplesPerSec = format.sampleRate;
	waveformat.wf.nAvgBytesPerSec = format.blockAlign * format.sampleRate;
	waveformat.wf.nBlockAlign = format.blockAlign;
	waveformat.wBitsPerSample = format.bitsPerSample;

	auto* result = new XAudio2SoundPlayer { *this, format, waveformat };
	if (result->sourceVoice == nullptr) {
		delete result;
		return nullptr;
	}
	return result;
}

#ifdef RHFW_PLATFORM_WINDOWSSTORE
#pragma comment( lib, "xaudio2.lib")
#endif /* RHFW_PLATFORM_WINDOWSSTORE */

#ifndef RHFW_PLATFORM_WINDOWSSTORE
static HRESULT WINAPI manual_XAudio2Create(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor) {
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
	return E_FAIL;
#else
	IXAudio2* pXAudio2;
	HRESULT res = CoCreateInstance(__uuidof(XAudio2), NULL, CLSCTX_INPROC_SERVER, __uuidof(IXAudio2), (void**) &pXAudio2);
	if (!SUCCEEDED(res)) {
		return res;
	}
	res = pXAudio2->Initialize(Flags, XAudio2Processor);
	if (SUCCEEDED(res)) {
		*ppXAudio2 = pXAudio2;
	} else {
		pXAudio2->Release();
	}
	return res;
#endif
}
#endif /* RHFW_PLATFORM_WINDOWSSTORE */

bool XAudio2AudioManager::loadImpl() {

	PROTO_XAudio2Create xaufunc_XAudio2Create;
#ifdef RHFW_PLATFORM_WINDOWSSTORE
	xaufunc_XAudio2Create = XAudio2Create;
#else
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
	libraryHandle = LoadLibraryA("XAudio2_9.dll");
	if (libraryHandle == NULL) {
		libraryHandle = LoadLibraryA("XAudio2_8.dll");
		if (libraryHandle == NULL) {
#endif
			libraryHandle = LoadLibraryA("XAudio2_7.dll");
			//TODO proper error handling
			ASSERT(libraryHandle != NULL) << "Failed to load XAudio2";
			if (libraryHandle == NULL) {
				return false;
			}
			libraryVersion = 7;
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
		} else {
			libraryVersion = 8;
		}
	} else {
		libraryVersion = 9;
	}
#endif

	if (libraryVersion >= 8) {
		xaufunc_XAudio2Create = (PROTO_XAudio2Create) GetProcAddress(libraryHandle, "XAudio2Create");
		ASSERT(xaufunc_XAudio2Create != nullptr);
	} else {
		xaufunc_XAudio2Create = manual_XAudio2Create;
	}

#endif /* RHFW_PLATFORM_WINDOWSSTORE */

	ThrowIfFailed(xaufunc_XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
	ThrowIfFailed(xaudio2->CreateMasteringVoice(&masterVoice));
	ASSERT(masterVoice != nullptr);
	return true;
}

void XAudio2AudioManager::freeImpl() {
	//TODO should destroy soundclips, otherwise DestroyVoice will fail.
	masterVoice->DestroyVoice();
	xaudio2->Release();
	xaudio2 = nullptr;
	masterVoice = nullptr;

#ifndef RHFW_PLATFORM_WINDOWSSTORE
	FreeLibrary(libraryHandle);
#endif /* RHFW_PLATFORM_WINDOWSSTORE */
}

XAudio2AudioManager::XAudio2AudioManager() {
}

XAudio2AudioManager::~XAudio2AudioManager() {
}

void XAudio2AudioManager::playMultiple(SoundClip * clips) {
	//TODO
	THROW()<< "unimplemented";
}

template<> audio::AudioManager* instantiateAudioManager<AudioConfig::XAudio2>() {
	return new XAudio2AudioManager { };
}

} // namespace audio
} // namespace rhfw
