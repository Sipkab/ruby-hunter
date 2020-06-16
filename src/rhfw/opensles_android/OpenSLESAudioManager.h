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
 * OpenSLESAudioManager.h
 *
 *  Created on: 2016. marc. 4.
 *      Author: sipka
 */

#ifndef OPENSLESAUDIOMANAGER_H_
#define OPENSLESAUDIOMANAGER_H_

#include <opensles_android/OpenSLESObjects.h>
#include <opensles_android/OpenSLESPlayer.h>
#include <framework/audio/AudioManager.h>
#include <framework/utils/LinkedList.h>
#include <framework/threading/Mutex.h>

namespace rhfw {
namespace audio {

class OpenSLESAudioManager: public AudioManager {
public:
	static inline SLint32 convertEndianness(Endianness e) {
		switch (e) {
			case Endianness::Big:
				return SL_BYTEORDER_BIGENDIAN;
			case Endianness::Little:
				return SL_BYTEORDER_LITTLEENDIAN;
			default: {
				THROW()<<"Unknown byteorder: " << e;
				return SL_BYTEORDER_LITTLEENDIAN;
			}
		}
	}
private:
	friend class OpenSLESPlayer;

	OpenSLESObject engineObject;
	OpenSLESObject outputMixObject;

	OpenSLESInterface<SLEngineItf> engineItf;

	virtual SoundPlayer* createSoundPlayerImpl(const SoundFormat& format) override;

	virtual bool loadImpl() override;
	virtual void freeImpl() override;
protected:
public:
	OpenSLESAudioManager();
	~OpenSLESAudioManager();

	virtual void playMultiple(SoundClip* clips) override;

};

}
 // namespace audio
}// namespace rhfw

#endif /* OPENSLESAUDIOMANAGER_H_ */
