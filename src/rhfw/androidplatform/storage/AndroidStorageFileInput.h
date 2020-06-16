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
 * AndroidStorageFileInput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef ANDROIDSTORAGEFILEINPUT_H_
#define ANDROIDSTORAGEFILEINPUT_H_

#include <androidplatform/AndroidFile.h>
#include <androidplatform/AndroidPlatform.h>
#include <framework/io/files/FileInput.h>

#include <gen/configuration.h>
#include <gen/log.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

namespace rhfw {

class AndroidStorageFileInput final: public FileInput {
private:
	friend class AndroidStorageFileDescriptor;

	FilePath path;
	int fd;

	virtual int readImpl(void* buffer, unsigned int length) override {
		ASSERT(fd >= 0) << "Invalid fd to read from";
		int res = ::read(fd, buffer, length);
//		LOGV() << "Read file data from: " << path.getURI() << ", count: " << length << ", result: " << res;
		ASSERT(res >= 0) << "Failed to read from file: " << path.getURI() << ", result: " << res << ", errno: " << strerror(errno);
		return res;
	}

	virtual bool openImpl() override {
		LOGV()<<"Open file at input path: " << path.getURI();
		fd = ::open(path.getURI(), O_RDONLY);
		if (fd < 0) {
			LOGW() << "Failed to open file for input: " << path.getURI() << ", errno: " << strerror(errno);
			return false;
		}
		return fd >= 0;
	}
	virtual void closeImpl() override {
		ASSERT(fd >= 0) << "Invalid fd to close";
		LOGV() << "Close file at input path: " << path.getURI();
		int res = ::close(fd);
		ASSERT(res == 0) << "closing file failed: " << strerror(errno);
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		off_t result = ::lseek(fd, offset, androidplatform::convertSeekMethod(method));
		ASSERT(result >= 0) << "lseek failed, errno: " << strerror(errno);
		return result;
	}

	static long long sizeFromFd(int fd) {
		struct stat st;
		int res = ::fstat(fd, &st);
		return res == 0 ? st.st_size : 0;
	}
public:
	AndroidStorageFileInput(FilePath path)
	: path(util::move(path)), fd(-1) {
	}
	AndroidStorageFileInput(AndroidStorageFileInput&& o)
	: FileInput(util::move(o)), path(util::move(o.path)), fd(o.fd) {
		o.fd = -1;
	}
	virtual ~AndroidStorageFileInput() {
		close();
	}

	virtual long long size() override {
		ASSERT(isOpened()) << "FileInput not opened";
		return sizeFromFd(fd);
	}

};

}
 // namespace rhfw

#endif /* ANDROIDSTORAGEFILEINPUT_H_ */
