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
 * AudioManager.h
 *
 *  Created on: 2016. marc. 2.
 *      Author: sipka
 */

#ifndef AUDIOMANAGER_H_
#define AUDIOMANAGER_H_

#include <framework/resource/TrackingResourceBlock.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/audio/SoundClip.h>
#include <framework/resource/ShareableResource.h>
#include <framework/audio/SoundPlayer.h>

#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {
namespace audio {

class SoundClip;
class SoundPlayer;
class AudioManager;

//TODO rework token system, its good for now
class SoundPlayerToken: private LinkedNode<SoundPlayerToken> {
	friend class SoundPlayer;
	friend class AudioManager;
public:
	class StoppedListener: public BasicListener<StoppedListener> {
	public:
		virtual void onPlayerStopped() = 0;
	};
private:
	LifeCycleChain<SoundPlayer, false> player;

	virtual SoundPlayerToken* get() override {
		return this;
	}

	void playerStopped() {
		for (auto&& l : stoppedListeners.foreach()) {
			l.onPlayerStopped();
		}
	}

	SoundPlayerToken(LinkedNode<SoundPlayer>& plr) {
		player.link(plr);
	}
public:
	StoppedListener::Events stoppedListeners;

	SoundPlayerToken() {
	}

	operator bool() {
		return player.get() != nullptr;
	}

	void stop() {
		this->removeLinkFromList();
		auto* plr = player.get();
		if (plr != nullptr) {
			plr->stop();
		}
	}

	SoundPlayer* getPlayer() {
		return player.get();
	}
};

class AudioManager: public ShareableResource {
private:
	friend class SoundPlayer;

	LinkedList<SoundClip, false> soundClips;

	core::time_millis streamDataMinDuration { 1000 };

	//try to create new SoundPlayer, return nullptr if failed
	virtual SoundPlayer* createSoundPlayerImpl(const SoundFormat& format) = 0;

	void onSoundPlayerStopped(SoundPlayer& player);

	virtual bool loadImpl() = 0;
	virtual void freeImpl() = 0;
protected:
	LinkedList<SoundPlayer, false> playingPlayersByPriority[(unsigned int) AudioPriority::_count_of_entries];
	LinkedList<SoundPlayer, false> stoppedPlayers;

	virtual bool load() override final;
	virtual void free() override final;
public:
	Resource<SoundClip> createSoundClip() {
		return Resource<SoundClip> { new TrackingResourceBlock<SoundClip> { new SoundClip { this }, soundClips } };
	}

	/**
	 * Starts playing multiple SoundClips, pointer is terminated by nullptr.
	 */
	virtual void playMultiple(SoundClip* clips) = 0;
	SoundPlayerToken playSingle(SoundClip& clip, float volumegain = 1.0f);

	const core::time_millis& getStreamDataMinDuration() const {
		return streamDataMinDuration;
	}
	void setStreamDataMinDuration(const core::time_millis& streamDataMinDuration) {
		this->streamDataMinDuration = streamDataMinDuration;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* AUDIOMANAGER_H_ */
