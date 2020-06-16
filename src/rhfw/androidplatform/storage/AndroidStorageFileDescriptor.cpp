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
 * AndroidFileDescriptor.cpp
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

AndroidStorageFileDescriptor::AndroidStorageFileDescriptor(FilePath path)
		: path { util::move(path) } {
}

AndroidStorageFileDescriptor::~AndroidStorageFileDescriptor() {
}

bool AndroidStorageFileDescriptor::exists() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0; //if doesnt exist, res != 0 && errno == ENOENT
}

bool AndroidStorageFileDescriptor::create() {
	return AndroidStorageFileOutput { path }.create();
}

bool AndroidStorageFileDescriptor::remove() {
	LOGV()<< "Delete file or directory at: " << path.getURI();
	int res = ::unlink(path.getURI());
	THROWIF(res != 0 && errno != ENOENT) << "::remove call failed result: " << res << ", errno: " << strerror(errno) << ", path: " << path.getURI();
	//return res == 0;
	return true;//if failed to remove, then its not there
}

bool AndroidStorageFileDescriptor::move(const StorageFileDescriptor& target) {
	int res = ::rename(path.getURI(), target.getPath().getURI());
	WARN(res != 0) << "Move call failed: " << path.getURI() << " to " << target.getPath().getURI();
	return res == 0;
}

/*
 bool AndroidStorageFileDescriptor::isDirectory() {
 struct stat st;
 int res = ::stat(path, &st);
 WARN(res != 0 && errno != ENOENT, "::stat call failed res: %d, errno: %s, path: %s", res, strerror(errno), path);
 return res == 0 && S_ISDIR(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
 }

 bool AndroidStorageFileDescriptor::isFile() {
 struct stat st;
 int res = ::stat(path, &st);
 WARN(res != 0 && errno != ENOENT, "::stat call failed res: %d, errno: %s, path: %s", res, strerror(errno), path);
 return res == 0 && S_ISREG(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
 }
 */

AndroidFd AndroidStorageFileDescriptor::openAndroidFd() {
	int result;
	result = ::open(path.getURI(), O_RDONLY);
	WARN(result < 0) << "Failed to open result: " << result << ", errno: " << strerror(errno) << ", path: " << path.getURI();
	if (result < 0) {
		return {result, 0, 0};
	}
	return {result, 0, AndroidStorageFileInput::sizeFromFd(result)};
}

long long AndroidStorageFileDescriptor::size() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed res: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0 ? st.st_size : -1;
}

}

