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
 * Win32CameraManager.h
 *
 *  Created on: 2016. dec. 10.
 *      Author: sipka
 */

#ifndef WIN32PLATFORM_CAMERA_WIN32CAMERAMANAGER_H_
#define WIN32PLATFORM_CAMERA_WIN32CAMERAMANAGER_H_

#include <framework/io/camera/CameraManager.h>

namespace rhfw {

class Win32CameraManager: public CameraManagerBase {
private:
	virtual Camera* openCameraImpl(const CameraInfo& info) override {
		return nullptr;
	}

	virtual bool loadImpl() override {
		return false;
	}
	virtual void freeImpl() override {
	}
public:
	virtual unsigned int getCameraCount() override {
		return 0;
	}
	virtual CameraInfo* getCameraInfos() override {
		return nullptr;
	}
};

}  // namespace rhfw

#endif /* WIN32PLATFORM_CAMERA_WIN32CAMERAMANAGER_H_ */
