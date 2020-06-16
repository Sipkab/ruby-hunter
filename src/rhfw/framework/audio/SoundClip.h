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
 * SoundClip.h
 *
 *  Created on: 2016. marc. 4.
 *      Author: sipka
 */

#ifndef SOUNDCLIP_H_
#define SOUNDCLIP_H_

#include <framework/audio/AudioObject.h>
#include <framework/audio/SoundFormat.h>
#include <framework/audio/StreamData.h>
#include <framework/audio/AudioDescriptor.h>

#include <framework/io/files/AssetFileDescriptor.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/core/timing.h>

#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {
namespace audio {

class SoundPlayer;
class AudioManager;
class SoundStreamer;
class AudioDescriptor;

class SoundClip: public AudioObject {
private:
	friend class SoundPlayer;
	friend class AudioManager;

	AudioPriority priority = AudioPriority::MEDIUM;

	//list of players that is playing this soundclip
	LinkedList<SoundPlayer, false> dependentPlayers;

	AudioManager* manager;

protected:
	AudioDescriptor* descriptor = nullptr;
	StreamData totalData;
	SoundFormat format;

	virtual bool load() override;
	virtual void free() override;
public:
	SoundClip(AudioManager* manager)
			: manager { manager } {
	}
	~SoundClip();

	void setPriority(AudioPriority priority) {
		this->priority = priority;
	}
	AudioPriority getPriority() const {
		return priority;
	}

	const SoundFormat& getFormat() const {
		return descriptor->getFormat();
	}

	void setDescriptor(AudioDescriptor* descriptor);

	core::time_millis getDuration() {
		return descriptor->getDuration();
	}

	SoundStreamer* createStreamer() {
		return descriptor->createStreamer();
	}
	const StreamData* getTotalData() {
		return totalData.getData() == nullptr ? nullptr : &totalData;
	}
};

}  // namespace audio
}  // namespace rhfw

#endif /* SOUNDCLIP_H_ */
