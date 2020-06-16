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
 * AndroidAssetDescriptor.h
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#ifndef ANDROIDASSETDESCRIPTOR_H_
#define ANDROIDASSETDESCRIPTOR_H_

#include <framework/io/files/AssetFileDescriptor.h>

#include <androidplatform/AndroidFile.h>
#include <androidplatform/asset/AndroidAssetFileInput.h>

#include <gen/configuration.h>

namespace rhfw {

class AndroidAssetDescriptor final: public AssetFileDescriptorBase, public AndroidFile {
private:
public:
	AndroidAssetDescriptor();
	AndroidAssetDescriptor(RAssetFile assetFileId);
	AndroidAssetDescriptor(AndroidAssetDescriptor&& o) = default;
	AndroidAssetDescriptor& operator=(AndroidAssetDescriptor&& o) = default;
	~AndroidAssetDescriptor() {
	}

	virtual AndroidAssetFileInput* createInput() override {
		return new AndroidAssetFileInput { assetFileId };
	}

	auto openInputStream() -> decltype(openAsStream(AndroidAssetFileInput {assetFileId})) {
		return openAsStream(AndroidAssetFileInput { assetFileId });
	}

	virtual AndroidFd openAndroidFd() override;

};

} // namespace rhfw

#endif /* ANDROIDASSETDESCRIPTOR_H_ */
