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
 * AppleStorageFileInput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef APPLESTORAGEFILEINPUT_H_
#define APPLESTORAGEFILEINPUT_H_

#include <appleplatform/ApplePlatform.h>
#include <framework/io/files/FileInput.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/log.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

namespace rhfw {

class AppleStorageFileInput final: public FileInput {
private:
	FilePath path;
	int fd = -1;

	virtual int readImpl(void* buffer, unsigned int length) override {
		ASSERT(fd >= 0) << "invalid file descriptor";
		int res = ::read(fd, buffer, length);
//		LOGV() << "Read file data from: " << path.getURI() << ", count: " << length << ", result: " << res;
		ASSERT(res >= 0) << "Failed to read from file: " << path.getURI() << ", result: " << res << ", errno: " << strerror(errno);
		return res;
	}

	virtual bool openImpl() override {
		LOGV()<<"Open file at input path: " << path.getURI();
		fd = ::open(path.getURI(), O_RDONLY);
		if (fd < 0) {
			LOGW() <<"Failed to open file for input: " << path.getURI() << ", errno: " << strerror(errno);
			return false;
		}
		return fd >= 0;
	}
	virtual void closeImpl() override {
		LOGV() << "Close file at input path: " << path.getURI();
		int res = ::close(fd);
		ASSERT(res == 0) << "closing file failed: " << strerror(errno);
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		off_t result = lseek(fd, offset, appleplatform::convertSeekMethod(method));
		ASSERT(result >= 0) << "lseek failed, errno: " << strerror(errno);
		return result;
	}
public:
	AppleStorageFileInput(FilePath path)
	: path {util::move(path)} {
	}
	AppleStorageFileInput(AppleStorageFileInput&& o)
	: FileInput {util::move(o)}, path {util::move(o.path)}, fd {o.fd} {
		o.fd = -1;
	}
	virtual ~AppleStorageFileInput() {
		close();
	}

	long long size() override {
		ASSERT(fd >= 0) << "invalid file descriptor";
		ASSERT(isOpened()) << "FileInput not opened";
		struct stat st;
		int res = ::stat(path.getURI(), &st);
		return res == 0 ? st.st_size : 0;
	}
};

}

#endif /* APPLESTORAGEFILEINPUT_H_ */
