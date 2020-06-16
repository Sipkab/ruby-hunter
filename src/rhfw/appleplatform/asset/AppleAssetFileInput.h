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
#ifndef APPLEASSETFILEINPUT_H_
#define APPLEASSETFILEINPUT_H_

#include <appleplatform/ApplePlatform.h>
#include <framework/io/files/FileInput.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>

#include <gen/log.h>
#include <gen/assets.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/syslimits.h>

namespace rhfw {

class AppleAssetFileInput: public FileInput {
private:
	RAssetFile assetId;

	int fd = -1;

	static bool getAssetFilePath(char * result, RAssetFile assetid, unsigned int maxLength);
protected:
	/**
	 * Return the bytes read.
	 * 0 on EOF, negative on ERROR, else the bytecount read.
	 */
	int readImpl(void* buffer, unsigned int length) override {
		ASSERT(fd >= 0) << "invalid file descriptor";
		int res = ::read(fd, buffer, length);
//		LOGV() << "Read file data from: " << assetId << ", count: " << length << ", result: " << res;
		ASSERT(res >= 0) << "Failed to read from file: " << assetId << ", result: " << res << ", errno: " << strerror(errno);
		return res;
	}

	bool openImpl() override {
		ASSERT(assetId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "Assetfileinput has invalid descriptor";
		char path[PATH_MAX];

		bool bresult = getAssetFilePath(path, assetId, sizeof path);

		if (!bresult)
			return false;

//		LOGV()<< "Open file at input path: " << path;
		fd = ::open(path, O_RDONLY);
		if (fd < 0) {
			LOGW()<< "Failed to open file for input: " << path << ", errno: " << strerror(errno);
			return false;
		}
		return fd >= 0;
	}
	void closeImpl() override {
//		LOGV()<<"Close file at input path: " << assetId;
		int res = ::close(fd);
		ASSERT(res == 0) << "closing file failed: " << strerror(errno);
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		off_t result = lseek(fd, offset, appleplatform::convertSeekMethod(method));
		ASSERT(result >= 0) << "lseek failed, errno: " << strerror(errno);
		return result;
	}
public:
	AppleAssetFileInput()
			: assetId(RAssetFile::INVALID_ASSET_IDENTIFIER) {
	}
	AppleAssetFileInput(RAssetFile assetId)
			: assetId(assetId) {
	}
	AppleAssetFileInput(AppleAssetFileInput&& o)
			: FileInput { util::move(o) }, assetId { o.assetId }, fd { o.fd } {
		o.fd = -1;
	}
	AppleAssetFileInput& operator=(AppleAssetFileInput&& o) {
		FileInput::operator=(util::move(o));

		this->assetId = o.assetId;
		this->fd = o.fd;

		o.assetId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.fd = -1;

		return *this;
	}
	virtual ~AppleAssetFileInput() {
		close();
	}

	long long size() override {
		ASSERT(fd >= 0) << "invalid file descriptor";
		ASSERT(isOpened()) << "FileInput not opened";
		struct stat st;
		int res = ::fstat(fd, &st);
		return res == 0 ? st.st_size : 0;
	}
};

}

#endif /*APPLEASSETFILEINPUT_H_*/
