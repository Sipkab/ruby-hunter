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
 * AppleStorageDirectoryDescriptor.cpp
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/utils/utility.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

namespace rhfw {

AppleStorageDirectoryDescriptor::AppleStorageDirectoryDescriptor(FilePath path)
		: path { util::move(path) } {
}

bool AppleStorageDirectoryDescriptor::create() {
	int res = ::mkdir(path.getURI(), S_IRWXU | S_IRWXG);
	THROWIF(res != 0 && errno != EEXIST) << "system call failed res: " << res << ", errno: " << strerror(errno) << " " << errno
			<< ", path: " << path.getURI();
	return res == 0 || errno == EEXIST;
}

bool AppleStorageDirectoryDescriptor::remove() {
	LOGV()<<"Delete file or directory at: " << path.getURI();
	int res = ::rmdir(path.getURI());
	THROWIF(res != 0 && errno != ENOENT) << "system call failed res: " << res << ", errno: " << strerror(errno) << " " << errno << ", path: " << path.getURI();
	return true; //if failed to remove, then its not there
}

bool AppleStorageDirectoryDescriptor::exists() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "system call failed res: " << res << ", errno: " << strerror(errno) << " " << errno
			<< ", path: " << path.getURI();
	return res == 0; //if doesnt exist, res != 0 && errno == ENOENT
}

class AppleFilePathEnumerator;
class AppleFilePathIterator: public DirectoryDescriptor::FilePathIterator {
	static bool isSkipFile(const char* path) {
		//skip paths '.', '..'
		return path[0] == '.' && (path[1] == 0 || (path[1] == '.' && path[2] == 0));
	}

	AppleFilePathEnumerator& enumerator;
	DIR* d;
	struct dirent *dir = nullptr;

	void goNextFile() {
		do {
			dir = readdir(d);
		} while (dir != nullptr && isSkipFile(dir->d_name));
	}
public:

	AppleFilePathIterator(AppleFilePathEnumerator& enumerator, DIR* d)
			: enumerator(enumerator), d { d }, dir { nullptr } {
		if (this->d != nullptr) {
			goNextFile();
		}
	}
	virtual ~AppleFilePathIterator() {
	}
	virtual bool operator !=(const DirectoryDescriptor::FilePathIterator& o) const override {
		const AppleFilePathIterator& it = static_cast<const AppleFilePathIterator&>(o);
		return dir != it.dir;
	}
	virtual AppleFilePathIterator& operator++() override {
		goNextFile();
		return *this;
	}
	virtual const FilePathChar* operator*() const override {
		WARN(dir->d_type == DT_UNKNOWN) << "Enumerating file type is undefined. d_type isn't implemented, for file: " << dir->d_name;
		return dir->d_name;
	}
	virtual bool isDirectory() const override;
};

class AppleFilePathEnumerator: public DirectoryDescriptor::FilePathEnumerator {
	friend class AppleFilePathIterator;

	FilePath path;
	DIR *d;
public:
	AppleFilePathEnumerator(const FilePath& path)
			: path { path } {
		d = opendir(path.getURI());
		WARN(d == nullptr) << "Failed to open directory stream: " << path.getURI() << ", errno: " << strerror(errno);
	}
	AppleFilePathEnumerator(AppleFilePathEnumerator&& o)
			: path(util::move(o.path)), d { o.d } {
		o.d = nullptr;
	}
	~AppleFilePathEnumerator() {
		if (d != nullptr) {
			int res = closedir(d);
			ASSERT(res == 0) << "Failed to close dir: " << strerror(errno);
		}
	}
	virtual DirectoryDescriptor::DirectoryIterator begin() override {
		return {new AppleFilePathIterator {*this, d}};
	}
	virtual DirectoryDescriptor::DirectoryIterator end() override {
		return {new AppleFilePathIterator {*this, nullptr}};
	}
};

bool AppleFilePathIterator::isDirectory() const {
	//d_type member might not be filled
	if (dir->d_type == DT_UNKNOWN) {
		struct stat st;
		const char* path = (enumerator.path + dir->d_name).getURI();
		int res = ::stat(path, &st);
		THROWIF(res != 0 && errno != EEXIST) << "system call failed res: " << res << ", errno: " << strerror(errno) << ", path: " << path;
		return res == 0 && S_ISDIR(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
	}
	return dir->d_type == DT_DIR;
}

DirectoryDescriptor::FilePathEnumerator* AppleStorageDirectoryDescriptor::createNewEnumerator() {
	return new AppleFilePathEnumerator { path };
}

}
