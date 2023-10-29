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
 * AndroidAssetDescriptor.cpp
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#include <framework/io/files/AssetFileDescriptor.h>

#include <android/asset_manager.h>

#include <gen/log.h>
#include <gen/configuration.h>

namespace rhfw {

AndroidAssetDescriptor::AndroidAssetDescriptor() {
}
AndroidAssetDescriptor::AndroidAssetDescriptor(RAssetFile assetFileId)
		: AssetFileDescriptorBase { assetFileId } {
	ASSERT(assetFileId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "Invalid file";
}

AndroidFd AndroidAssetDescriptor::openAndroidFd() {
	AAssetManager* am = androidplatform::getAssetManager();
	ASSERT(am != nullptr) << "assetmanager is nullptr";

	char path[64];
	//TODO solve uncompressed data
	snprintf(path, sizeof(path), "res/%x", (unsigned int) assetFileId);

	AAsset * file = AAssetManager_open(am, path, AASSET_MODE_UNKNOWN);
	ASSERT(file != nullptr) << "Failed to open file: " << path;

	off_t start;
	off_t length;
	int result = AAsset_openFileDescriptor(file, &start, &length);

	AAsset_close(file);
	WARN(result < 0) << "Failed to open asset as fd: " << path;

	if (result < 0) {
		return {result, 0, 0};
	}

	return {result, start, length};
}
}

