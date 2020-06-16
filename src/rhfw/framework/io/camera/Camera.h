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
 * Camera.h
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_CAMERA_CAMERA_H_
#define FRAMEWORK_IO_CAMERA_CAMERA_H_

#include <framework/io/camera/CameraInfo.h>
#include <framework/utils/LinkedNode.h>

#include <gen/platform.h>

namespace rhfw {
class CameraManagerBase;

class CameraBase: private LinkedNode<CameraBase> {
	friend class CameraManagerBase;

	CameraInfo info;

	virtual CameraBase* get() override {
		return this;
	}
public:
	CameraBase(const CameraInfo& info)
			: info(info) {
	}
	virtual ~CameraBase() = default;

	const CameraInfo& getInfo() const {
		return info;
	}

};

} // namespace rhfw

#include CAMERA_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class CAMERA_EXACT_CLASS_TYPE Camera;
} // namespace rhfw

#endif /* FRAMEWORK_IO_CAMERA_CAMERA_H_ */
