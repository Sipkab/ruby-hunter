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
 * SoundPlayer.h
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_SOUNDPLAYER_H_
#define FRAMEWORK_AUDIO_SOUNDPLAYER_H_

#include <framework/audio/SoundClip.h>
#include <framework/audio/SoundFormat.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/MemberLinkedNode.h>

namespace rhfw {
namespace audio {

class SoundClip;
class SoundFormat;
class SoundPlayerToken;

class SoundPlayer: public LinkedNode<SoundPlayer> {
	friend class AudioManager;
	friend class SoundClip;
	friend class SoundPlayerToken;
protected:
	AudioManager& manager;
private:

	LifeCycleChain<SoundPlayerToken, false> token;
	MemberLinkedNode<SoundPlayer> tokenNode { this };
	MemberLinkedNode<SoundPlayer> clipNode { this };
	SoundClip* currentClip = nullptr;

	SoundFormat format;

	bool manualPause = false;

	float volumeGain = 1.0f;
protected:
	//call it in subclass, when the playing of the enqueued SoundClip finished
	void onPlayerStopped();

	virtual bool play(SoundClip& clip) = 0;
	virtual void stop() = 0;
	virtual bool stopAndPlay(SoundClip& clip) {
		stop();
		return play(clip);
	}

	virtual bool setVolumeGainImpl(float gain) = 0;
public:
	SoundPlayer(AudioManager& manager, const SoundFormat& format)
			: manager(manager), format(format) {
	}
	virtual ~SoundPlayer() = default;

	SoundClip* getCurrentClip() {
		return currentClip;
	}

	const SoundFormat& getFormat() const {
		return format;
	}
	SoundPlayer* get() override {
		return this;
	}

	void setVolumeGain(float gain);
	float getVolumeGain() const {
		return volumeGain;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_SOUNDPLAYER_H_ */
