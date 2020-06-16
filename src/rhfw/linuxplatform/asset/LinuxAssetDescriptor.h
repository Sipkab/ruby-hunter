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
 * LinuxAssetDescriptor.h
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#ifndef LINUXASSETDESCRIPTOR_H_
#define LINUXASSETDESCRIPTOR_H_

#include <framework/io/files/AssetFileDescriptor.h>

#include <linuxplatform/asset/LinuxAssetFileInput.h>

#include <gen/configuration.h>

namespace rhfw {

class LinuxAssetDescriptor final: public AssetFileDescriptorBase {
private:
public:
	LinuxAssetDescriptor() {
	}
	LinuxAssetDescriptor(RAssetFile assetid)
			: AssetFileDescriptorBase(assetid) {
	}
	LinuxAssetDescriptor(LinuxAssetDescriptor&& o) = default;
	LinuxAssetDescriptor& operator=(LinuxAssetDescriptor&& o) = default;
	~LinuxAssetDescriptor() {
	}

	virtual LinuxAssetFileInput* createInput() override {
		return new LinuxAssetFileInput { assetFileId };
	}

	auto openInputStream() -> decltype(openAsStream(LinuxAssetFileInput {assetFileId})) {
		return openAsStream(LinuxAssetFileInput { assetFileId });
	}

};

} // namespace rhfw

#endif /* LINUXASSETDESCRIPTOR_H_ */
