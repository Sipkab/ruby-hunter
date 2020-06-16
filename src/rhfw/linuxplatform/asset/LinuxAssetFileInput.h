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
 * LinuxAssetFileInput.h
 *
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#ifndef LINUXPLATFORM_ASSET_LINUXASSETFILEINPUT_H_
#define LINUXPLATFORM_ASSET_LINUXASSETFILEINPUT_H_

#include <linuxplatform/LinuxPlatform.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/files/FilePath.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gen/log.h>

namespace rhfw {

class LinuxAssetFileInput final: public FileInput {
private:
	friend class LinuxAssetFileDescriptor;

	RAssetFile assetFileId = RAssetFile::INVALID_ASSET_IDENTIFIER;
	int fd = -1;

	virtual int readImpl(void* buffer, unsigned int length) override {
		ASSERT(fd >= 0) << "Invalid fd to read from";
		int res = ::read(fd, buffer, length);
//		LOGV()<< "Read file data from: " << assetFileId << ", count: " << length << ", result: " << res;
		ASSERT(res >= 0) << "Failed to read from file: " << assetFileId << ", result: " << res << ", errno: " << strerror(errno);
		return res;
	}

	virtual bool openImpl() override {
		ASSERT(assetFileId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "Invalid file to open";
//		LOGV()<< "Open file at input path: " << assetFileId;
		char pathbuf[11 + 8 + 1];
		sprintf(pathbuf, "assets/res/%x", (unsigned int) assetFileId);

		FilePath path = linuxplatform::getAssetsPath() + pathbuf;

		fd = ::open(path.getURI(), O_RDONLY);
		if (fd < 0) {
			LOGW()<< "Failed to open file for input: " << assetFileId << ", errno: " << strerror(errno);
			return false;
		}
		return fd >= 0;
	}
	virtual void closeImpl() override {
		ASSERT(fd >= 0) << "Invalid fd to close";
//		LOGV()<< "Close file at input path: " << assetFileId;
		int res = ::close(fd);
		ASSERT(res == 0) << "closing file failed: " << strerror(errno);
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		off_t result = ::lseek(fd, offset, linuxplatform::convertSeekMethod(method));
		ASSERT(result >= 0) << "lseek failed, errno: " << strerror(errno);
		return result;
	}

	static long long sizeFromFd(int fd) {
		struct stat st;
		int res = ::fstat(fd, &st);
		return res == 0 ? st.st_size : 0;
	}
public:
	LinuxAssetFileInput(RAssetFile assetid)
			: assetFileId { assetid }, fd { -1 } {
	}
	LinuxAssetFileInput(LinuxAssetFileInput&& o)
			: FileInput { util::move(o) }, assetFileId { o.assetFileId }, fd { o.fd } {
		o.fd = -1;
	}
	virtual ~LinuxAssetFileInput() {
		close();
	}

	virtual long long size() override {
		ASSERT(isOpened()) << "FileInput not opened";
		return sizeFromFd(fd);
	}

};

} // namespace rhfw

#endif /* LINUXPLATFORM_ASSET_LINUXASSETFILEINPUT_H_ */
