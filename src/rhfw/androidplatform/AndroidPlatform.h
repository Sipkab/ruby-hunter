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
 * AndroidPlatform.h
 *
 *  Created on: 2014.06.07.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_H_
#define ANDROIDPLATFORM_H_

#include <unistd.h>
#include <gen/types.h>
#include <gen/log.h>

#include <jni.h>

struct AAssetManager;

namespace rhfw {
namespace androidplatform {

AAssetManager* getAssetManager();
unsigned int getSdkVersion();

//XXX temporary
extern jclass nativeActivityClass;

inline static int convertSeekMethod(SeekMethod method) {
	switch (method) {
		case SeekMethod::BEGIN: {
			return SEEK_SET;
		}
		case SeekMethod::CURRENT: {
			return SEEK_CUR;
		}
		case SeekMethod::END: {
			return SEEK_END;
		}
		default: {
			THROW()<<"Invalid seek method " << method;
			return SEEK_SET;
		}
	}
}

}
 // namespace android
}// namespace rhfw

#endif /* ANDROIDPLATFORM_H_ */
