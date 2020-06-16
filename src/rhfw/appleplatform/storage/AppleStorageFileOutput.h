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
 * AppleStorageFileOutput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef APPLESTORAGEFILEOUTPUT_H_
#define APPLESTORAGEFILEOUTPUT_H_

#include <framework/io/files/FileOutput.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/log.h>

#include <fcntl.h>
#include <sys/stat.h>

namespace rhfw {

class AppleStorageFileOutput final: public FileOutput {
private:
	FilePath path;
	int fd = -1;

	virtual int writeImpl(const void* data, int count) override {
		ASSERT(fd >= 0) << "invalid file descriptor";
//		LOGV() << "Write file data to: " << path.getURI() << ", count: " << count;
		int res = ::write(fd, data, count);
		ASSERT(res > 0) << "Failed to write to file: " << path.getURI() << ", count: " << count << ", res: " << res << ", errno: ", strerror(
				errno);
		return res;
	}

	virtual bool openImpl() override {
		LOGV()<<"Open file for output path: " << path.getURI();
		fd = ::open(path.getURI(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0) {
			LOGW() << "Failed to open file for output: " << path.getURI() << ", errno: " << strerror(errno);
			return false;
		}
		if (isAppend()) {
			off_t res = ::lseek(fd, 0, SEEK_END);
			THROWIF(res < 0) << "Failed to seek file: " << path.getURI() << ", errno: " << strerror(errno);
		}

		return true;
	}
	virtual void closeImpl() override {
		LOGV() << "Close file for output path: " << path.getURI();
		int res = ::close(fd);
		ASSERT(res == 0) << "closing file failed: " << strerror(errno);
	}

	virtual bool flushDiskImpl() override {
		ASSERT(fd >= 0) << "invalid file descriptor";
		int res = ::fsync(fd);
		ASSERT(res == 0) << "flushing file failed: " << strerror(errno);
		return res == 0;
	}
public:
	AppleStorageFileOutput(FilePath path)
	: path {util::move(path)} {
	}
	AppleStorageFileOutput(AppleStorageFileOutput&& o)
	: FileOutput {util::move(o)}, path {util::move(o.path)}, fd {o.fd} {
		o.fd = -1;
	}
	virtual ~AppleStorageFileOutput() {
		close();
	}

};

}

#endif /* APPLESTORAGEFILEOUTPUT_H_ */
