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
 * LinuxPlatform.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_LINUXPLATFORM_H_
#define LINUXPLATFORM_LINUXPLATFORM_H_

#include <gen/types.h>
#include <gen/log.h>
#include <unistd.h>

namespace rhfw {
class FilePath;
namespace linuxplatform {

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
			THROW()<< "Invalid seek method " << method;
			return SEEK_SET;
		}
	}
}

const FilePath& getAssetsPath();

}  // namespace linuxplatform
}  // namespace rhfw

#endif /* LINUXPLATFORM_LINUXPLATFORM_H_ */
