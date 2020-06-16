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
 * CameraManager.cpp
 *
 *  Created on: 2016. dec. 9.
 *      Author: sipka
 */

#include <framework/io/camera/CameraManager.h>

#include <gen/log.h>

namespace rhfw {

CameraManagerBase::CameraManagerBase() {
}

CameraManagerBase::~CameraManagerBase() {
}

bool CameraManagerBase::reload() {
	THROW()<<"Invalid call for CameraManager";
	return false;
}

Camera* CameraManagerBase::openCamera(const CameraInfo& info) {
	Camera* cam = openCameraImpl(info);
	if (cam != nullptr) {
		cameras.addToEnd(*cam);
	}
	return cam;
}

Camera* CameraManagerBase::openDefaultCamera(CameraFacing facing) {
	unsigned int count = getCameraCount();
	CameraInfo* infos = getCameraInfos();
	for (unsigned int i = 0; i < count; ++i) {
		if (infos[i].facing == facing) {
			auto* res = openCamera(infos[i]);
			if (res != nullptr) {
				return res;
			}
		}
	}
	return nullptr;
}

void CameraManagerBase::free() {
	cameras.clear();
	freeImpl();
}
bool CameraManagerBase::load() {
	return loadImpl();
}

} // namespace rhfw

