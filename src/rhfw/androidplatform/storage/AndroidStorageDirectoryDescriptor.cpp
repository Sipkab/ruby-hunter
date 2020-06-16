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
 * AndroidStorageDirectoryDescriptor.cpp
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/utils/utility.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

namespace rhfw {

AndroidStorageDirectoryDescriptor::AndroidStorageDirectoryDescriptor(FilePath path)
		: path { util::move(path) } {
}

bool AndroidStorageDirectoryDescriptor::create() {
	int res = ::mkdir(path.getURI(), S_IRWXU | S_IRWXG);
	THROWIF(res != 0 && errno != EEXIST) << "create directory result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0 || errno == EEXIST;
}

bool AndroidStorageDirectoryDescriptor::remove() {
	LOGV()<< "Delete file or directory at: " << path.getURI();
	int res = ::rmdir(path.getURI());
	THROWIF(res != 0 && errno != ENOENT) << "::remove call failed result: " << res << ", errno: " << strerror(errno) << ", path: " << path.getURI();
	//return res == 0;
	return true;//if failed to remove, then its not there
}

bool AndroidStorageDirectoryDescriptor::exists() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0; //if doesnt exist, res != 0 && errno == ENOENT
}

class AndroidFilePathEnumerator;
class AndroidFilePathIterator: public DirectoryDescriptor::FilePathIterator {
	static bool isSkipFile(const char* path) {
		//skip paths '.', '..'
		return path[0] == '.' && (path[1] == 0 || (path[1] == '.' && path[2] == 0));
	}

	AndroidFilePathEnumerator& enumerator;
	DIR* d;
	struct dirent *dir = nullptr;

	void goNextFile() {
		do {
			dir = readdir(d);
		} while (dir != nullptr && isSkipFile(dir->d_name));
	}
public:

	AndroidFilePathIterator(AndroidFilePathEnumerator& enumerator, DIR* d)
			: enumerator(enumerator), d { d }, dir { nullptr } {
		if (this->d != nullptr) {
			goNextFile();
		}
	}
	virtual ~AndroidFilePathIterator() {
	}
	virtual bool operator !=(const DirectoryDescriptor::FilePathIterator& o) const override {
		const AndroidFilePathIterator& it = static_cast<const AndroidFilePathIterator&>(o);
		return dir != it.dir;
	}
	virtual AndroidFilePathIterator& operator++() override {
		goNextFile();
		return *this;
	}
	virtual const FilePathChar* operator*() const override {
		WARN(dir->d_type == DT_UNKNOWN) << "Enumerating file type is undefined. d_type isn't implemented, for file: " << dir->d_name;
		return dir->d_name;
	}
	virtual bool isDirectory() const override;
};

class AndroidFilePathEnumerator: public DirectoryDescriptor::FilePathEnumerator {
	friend class AndroidFilePathIterator;

	FilePath path;
	DIR *d;
public:
	AndroidFilePathEnumerator(const FilePath& path)
			: path { path } {
		d = opendir(path.getURI());
		WARN(d == nullptr) << "Failed to open directory stream: " << path.getURI() << ", errno: " << strerror(errno);
	}
	AndroidFilePathEnumerator(AndroidFilePathEnumerator&& o)
			: path(util::move(o.path)), d { o.d } {
		o.d = nullptr;
	}
	~AndroidFilePathEnumerator() {
		if (d != nullptr) {
			int res = closedir(d);
			ASSERT(res == 0) << "Failed to close dir: " << strerror(errno);
		}
	}
	virtual DirectoryDescriptor::DirectoryIterator begin() override {
		return {new AndroidFilePathIterator {*this, d}};
	}
	virtual DirectoryDescriptor::DirectoryIterator end() override {
		return {new AndroidFilePathIterator {*this, nullptr}};
	}
};

bool AndroidFilePathIterator::isDirectory() const {
	//d_type member might not be filled
	if (dir->d_type == DT_UNKNOWN) {
		struct stat st;
		const char* path = (enumerator.path + (const char*) dir->d_name).getURI();
		int res = ::stat(path, &st);
		WARN(res != 0 && errno != ENOENT) << "::stat call failed result: " << res << ", errno: " << strerror(errno) << ", path: "
				<< path;
		return res == 0 && S_ISDIR(st.st_mode); //if doesnt exist, res != 0 && errno == ENOENT
	}
	return dir->d_type == DT_DIR;
}

DirectoryDescriptor::FilePathEnumerator* AndroidStorageDirectoryDescriptor::createNewEnumerator() {
	return new AndroidFilePathEnumerator { path };
}
}
