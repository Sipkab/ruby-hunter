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
 * AndroidAssetFileInput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef ANDROIDASSETFILEINPUT_H_
#define ANDROIDASSETFILEINPUT_H_

#include <framework/io/files/FileInput.h>
#include <framework/utils/utility.h>

#include <android/asset_manager.h>
#include <unistd.h>
#include <stdio.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <androidplatform/AndroidPlatform.h>

namespace rhfw {

class AndroidAssetFileInput final: public FileInput {
private:
	RAssetFile assetFileId;

	AAsset* file = nullptr;

	virtual int readImpl(void* buffer, unsigned int length) override {
		ASSERT(file != nullptr) << "FileInput is not opened: " << assetFileId;
		return AAsset_read(file, buffer, length);
	}

	virtual bool openImpl() override {
		ASSERT(file == nullptr) << "FileInput already opened";
		ASSERT(assetFileId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "invalid assetfile";

		AAssetManager* am = androidplatform::getAssetManager();
		ASSERT(am != nullptr) << "assetmanager is nullptr";

		char path[32];
		sprintf(path, "res/%x", (unsigned int) assetFileId);

		file = AAssetManager_open(am, path, AASSET_MODE_UNKNOWN);
		ASSERT(file != nullptr) << "Failed to open file: " << assetFileId;

		return file != nullptr;
	}
	virtual void closeImpl() override {
		ASSERT(file != nullptr) << "closing with nullptr file";

		AAsset_close(file);
		file = nullptr;
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		ASSERT(file != nullptr) << "FileInput is not opened: " << assetFileId;
		return AAsset_seek(file, offset, androidplatform::convertSeekMethod(method));
	}
public:
	AndroidAssetFileInput(RAssetFile assetFileId)
			: assetFileId { assetFileId } {
	}
	AndroidAssetFileInput(AndroidAssetFileInput&& o)
			: FileInput(util::move(o)), assetFileId { o.assetFileId }, file { o.file } {
		o.assetFileId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = nullptr;
	}
	AndroidAssetFileInput& operator=(AndroidAssetFileInput&& o) {
		FileInput::operator =(util::move(o));

		this->assetFileId = o.assetFileId;
		this->file = o.file;

		o.assetFileId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = nullptr;
		return *this;
	}
	virtual ~AndroidAssetFileInput() {
		close();
	}

	virtual long long size() override {
		ASSERT(isOpened()) << "FileInput not opened " << assetFileId;
		return AAsset_getLength(file);
	}

};

}

#endif /* ANDROIDASSETFILEINPUT_H_ */
