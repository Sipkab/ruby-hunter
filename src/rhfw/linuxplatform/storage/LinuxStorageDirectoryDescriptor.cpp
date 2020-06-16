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
 * LinuxStorageDirectoryDescriptor.cpp
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/utils/utility.h>
#include <framework/utils/FixedString.h>
#include <linuxplatform/LinuxPlatform.h>

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>

#include <gen/configuration.h>

namespace rhfw {
static FilePath assetsPath;
namespace linuxplatform {
const FilePath& getAssetsPath() {
	return assetsPath;
}
} // namespace linuxplatform

static FilePath LinuxAppDataRoot;
const FilePath& LinuxStorageDirectoryDescriptor::ApplicationDataRoot() {
	return LinuxAppDataRoot;
}
FilePath LinuxStorageDirectoryDescriptor::ApplicationDataDirectory(const char* appname) {
	return LinuxAppDataRoot + (const char*) (FixedString { "." } + appname);
}

void LinuxStorageDirectoryDescriptor::InitializePlatformRootDirectory(int argc, char* argv[]) {
	auto* home = getenv("HOME");
	ASSERT(home != nullptr) << "HOME environment variable is missing";
	LinuxAppDataRoot = FilePath { home };
	StorageDirectoryDescriptor::initializeRootDirectory(LinuxAppDataRoot + "." APPLICATION_DEFAULT_NAME);
	StorageDirectoryDescriptor { StorageDirectoryDescriptor::Root() }.create();

	char buffer[512];
	auto rlres1 = readlink("/proc/self/exe", buffer, sizeof(buffer));
	ASSERT(rlres1 >= 0);
	if (rlres1 >= 0) {
		auto len = rlres1;
		while (len > 0 && buffer[len - 1] != FILE_SEPARATOR_CHAR) {
			--len;
		}
		if (len > 0) {
			//found a file separator
			buffer[len - 1] = 0;
			--len;
		} else {
			//else len == 0
			buffer[0] = '.';
			buffer[1] = 0;
			len = 1;
		}
		assetsPath = FilePath { buffer, (unsigned int) len };
	} else {
		//try the first argument, as exe path
		if (argc >= 1) {
			unsigned int len = strlen(argv[0]);
			memcpy(buffer, argv[0], len + 1);
			while (len > 0 && buffer[len - 1] != FILE_SEPARATOR_CHAR) {
				--len;
			}
			if (len > 0) {
				//found a file separator
				buffer[len - 1] = 0;
				--len;
			} else {
				//else len == 0
				buffer[0] = '.';
				buffer[1] = 0;
				len = 1;
			}
			assetsPath = FilePath { buffer, (unsigned int) len };
		} else {
			assetsPath = FilePath { ".", 1 };
		}
	}

	LOGI()<< "Executable directory: " << buffer;

}

LinuxStorageDirectoryDescriptor::LinuxStorageDirectoryDescriptor(FilePath path)
		: path { util::move(path) } {
}

bool LinuxStorageDirectoryDescriptor::create() {
	int res = ::mkdir(path.getURI(), S_IRWXU | S_IRWXG);
	THROWIF(res != 0 && errno != EEXIST) << "create directory result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0 || errno == EEXIST;
}

bool LinuxStorageDirectoryDescriptor::remove() {
	LOGV()<< "Delete file or directory at: " << path.getURI();
	int res = ::rmdir(path.getURI());
	THROWIF(res != 0 && errno != ENOENT) << "::remove call failed result: " << res << ", errno: " << strerror(errno) << ", path: " << path.getURI();
	//return res == 0;
	return true;//if failed to remove, then its not there
}

bool LinuxStorageDirectoryDescriptor::exists() {
	struct stat st;
	int res = ::stat(path.getURI(), &st);
	THROWIF(res != 0 && errno != ENOENT) << "::stat call failed result: " << res << ", errno: " << strerror(errno) << ", path: "
			<< path.getURI();
	return res == 0; //if doesnt exist, res != 0 && errno == ENOENT
}

class LinuxFilePathEnumerator;
class LinuxFilePathIterator: public DirectoryDescriptor::FilePathIterator {
	static bool isSkipFile(const char* path) {
		//skip paths '.', '..'
		return path[0] == '.' && (path[1] == 0 || (path[1] == '.' && path[2] == 0));
	}

	LinuxFilePathEnumerator& enumerator;
	DIR* d;
	struct dirent *dir = nullptr;

	void goNextFile() {
		do {
			dir = readdir(d);
		} while (dir != nullptr && isSkipFile(dir->d_name));
	}
public:

	LinuxFilePathIterator(LinuxFilePathEnumerator& enumerator, DIR* d)
			: enumerator(enumerator), d { d }, dir { nullptr } {
		if (this->d != nullptr) {
			goNextFile();
		}
	}
	virtual ~LinuxFilePathIterator() {
	}
	virtual bool operator !=(const DirectoryDescriptor::FilePathIterator& o) const override {
		const LinuxFilePathIterator& it = static_cast<const LinuxFilePathIterator&>(o);
		return dir != it.dir;
	}
	virtual LinuxFilePathIterator& operator++() override {
		goNextFile();
		return *this;
	}
	virtual const FilePathChar* operator*() const override {
		WARN(dir->d_type == DT_UNKNOWN) << "Enumerating file type is undefined. d_type isn't implemented, for file: " << dir->d_name;
		return dir->d_name;
	}
	virtual bool isDirectory() const override;
};

class LinuxFilePathEnumerator: public DirectoryDescriptor::FilePathEnumerator {
	friend class LinuxFilePathIterator;

	FilePath path;
	DIR *d;
public:
	LinuxFilePathEnumerator(const FilePath& path)
			: path { path } {
		d = opendir(path.getURI());
		WARN(d == nullptr) << "Failed to open directory stream: " << path.getURI() << ", errno: " << strerror(errno);
	}
	LinuxFilePathEnumerator(LinuxFilePathEnumerator&& o)
			: path(util::move(o.path)), d { o.d } {
		o.d = nullptr;
	}
	~LinuxFilePathEnumerator() {
		if (d != nullptr) {
			int res = closedir(d);
			ASSERT(res == 0) << "Failed to close dir: " << strerror(errno);
		}
	}
	virtual DirectoryDescriptor::DirectoryIterator begin() override {
		return {new LinuxFilePathIterator {*this, d}};
	}
	virtual DirectoryDescriptor::DirectoryIterator end() override {
		return {new LinuxFilePathIterator {*this, nullptr}};
	}
};

bool LinuxFilePathIterator::isDirectory() const {
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

DirectoryDescriptor::FilePathEnumerator* LinuxStorageDirectoryDescriptor::createNewEnumerator() {
	return new LinuxFilePathEnumerator { path };
}
} // namespace rhfw
