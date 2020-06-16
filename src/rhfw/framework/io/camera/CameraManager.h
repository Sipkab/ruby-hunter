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
 * CameraManager.h
 *
 *  Created on: 2016. dec. 9.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_CAMERA_CAMERAMANAGER_H_
#define FRAMEWORK_IO_CAMERA_CAMERAMANAGER_H_

#include <framework/resource/ShareableResource.h>

#include <framework/io/camera/Camera.h>
#include <framework/io/camera/CameraInfo.h>
#include <framework/utils/LinkedList.h>

#include <gen/fwd/types.h>
#include <gen/platform.h>

namespace rhfw {

class CameraManagerBase: public ShareableResource {
private:
	LinkedList<CameraBase> cameras;

	virtual Camera* openCameraImpl(const CameraInfo& info) = 0;

	virtual bool loadImpl() = 0;
	virtual void freeImpl() = 0;
public:
	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;

	CameraManagerBase();
	~CameraManagerBase();

	virtual unsigned int getCameraCount() = 0;
	virtual CameraInfo* getCameraInfos() = 0;

	Camera* openCamera(const CameraInfo& info);

	Camera* openDefaultCamera(CameraFacing facing);
};

} // namespace rhfw

#include CAMERAMANAGER_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class CAMERAMANAGER_EXACT_CLASS_TYPE CameraManager;
} // namespace rhfw

#endif /* FRAMEWORK_IO_CAMERA_CAMERAMANAGER_H_ */
