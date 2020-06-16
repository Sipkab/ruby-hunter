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
 * LinuxFileDescriptor.cpp
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/files/FileOutput.h>
#include <framework/utils/utility.h>

#include <gen/log.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace rhfw {

LinuxStorageFileDescriptor::LinuxStorageFileDescriptor(FilePath path)
		: path { util::move(path) } {
}

LinuxStorageFileDescriptor::~LinuxStorageFileDescriptor() {
}

bool LinuxStorageFileDescriptor::exists() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0; //if doesnt exist, res != 0 && errno == ENOENT
}

bool LinuxStorageFileDescriptor::create() {
	return LinuxStorageFileOutput { path }.create();
}

bool LinuxStorageFileDescriptor::remove() {
	LOGV()<<"Delete file or directory at: " << path.getURI();
	int res = ::unlink(path.getURI());
	THROWIF(res != 0 && errno != ENOENT) << "::remove call failed result: " << res << ", errno: " << strerror(errno) << ", path: " << path.getURI();
	//return res == 0;
	return true;//if failed to remove, then its not there
}

bool LinuxStorageFileDescriptor::move(const StorageFileDescriptor& target) {
	int res = ::rename(path.getURI(), target.getPath().getURI());
	WARN(res != 0) << "Move call failed: " << path.getURI() << " to " << target.getPath().getURI();
	return res == 0;
}

/*
 bool LinuxStorageFileDescriptor::isDirectory() {
 struct stat st;
 int res = ::stat(path, &st);
 WARN(res != 0 && errno != ENOENT, "::stat call failed res: %d, errno: %s, path: %s", res, strerror(errno), path);
 return res == 0 && S_ISDIR(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
 }

 bool LinuxStorageFileDescriptor::isFile() {
 struct stat st;
 int res = ::stat(path, &st);
 WARN(res != 0 && errno != ENOENT, "::stat call failed res: %d, errno: %s, path: %s", res, strerror(errno), path);
 return res == 0 && S_ISREG(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
 }
 */

long long LinuxStorageFileDescriptor::lastModified() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	WARN(res != 0) << "syscall failed: " << strerror(errno) << ", path: " << path.getURI();
	if (res != 0) {
		return -1;
	}
	return (long long) st.st_mtim.tv_sec * 1000 + (long long) st.st_mtim.tv_nsec / 1000000;
}
long long LinuxStorageFileDescriptor::size() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed res: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0 ? st.st_size : -1;
}

} // namespace rhfw

