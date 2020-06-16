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
#ifndef XAUDIO2AUDIOMANAGER_H_
#define XAUDIO2AUDIOMANAGER_H_

#include <framework/audio/AudioManager.h>
#include <xaudio2/xaudio2_header.h>
#include <gen/platform.h>

namespace rhfw {
namespace audio {

class XAudio2AudioManager: public AudioManager {
private:
	typedef HRESULT (WINAPI *PROTO_XAudio2Create)(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);
	virtual SoundPlayer* createSoundPlayerImpl(const SoundFormat& format) override;

#ifndef RHFW_PLATFORM_WINDOWSSTORE
	HMODULE libraryHandle = NULL;
	int libraryVersion = -1;
#endif /* RHFW_PLATFORM_WINDOWSSTORE */

	IXAudio2* xaudio2 = nullptr;
	IXAudio2MasteringVoice* masterVoice = nullptr;

	UINT32 operationSetCounter = 1;

	virtual bool loadImpl() override;
	virtual void freeImpl() override;
protected:
public:
	XAudio2AudioManager();
	~XAudio2AudioManager();

	IXAudio2* getXAudio2() {
		return xaudio2;
	}
	IXAudio2MasteringVoice* getMasteringVoice() {
		return masterVoice;
	}
	virtual void playMultiple(SoundClip* clips) override;

	UINT32 getNewOperationSet() {
		return operationSetCounter++;
	}
};

} // namespace audio
} // namespace rhfw
#endif /*XAUDIO2AUDIOMANAGER_H_*/
