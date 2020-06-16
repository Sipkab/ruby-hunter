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
 * AudioObject.h
 *
 *  Created on: 2016. apr. 2.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_AUDIOOBJECT_H_
#define FRAMEWORK_AUDIO_AUDIOOBJECT_H_

#include <framework/resource/ShareableResource.h>

namespace rhfw {
namespace audio {

class AudioObject: public ShareableResource {
protected:
public:
};

}  // namespace audio
}  // namespace rhfw

#endif /* FRAMEWORK_AUDIO_AUDIOOBJECT_H_ */
