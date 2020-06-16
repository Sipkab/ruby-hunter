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
 * Win32StorageDirectoryDescriptor.cpp
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/utils/utility.h>

#include <Windows.h>
#include <Shlobj.h>
#include <Objbase.h>

#include <gen/log.h>
#include <gen/types.h>
#include <gen/configuration.h>

#include <cstring>
#include <string.h>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

namespace rhfw {

FilePath Win32StorageDirectoryDescriptor::STORAGE_ASSETS_ROOT_DIR;

static FilePath Win32AppDataRoot;
const FilePath& Win32StorageDirectoryDescriptor::ApplicationDataRoot() {
	return Win32AppDataRoot;
}
FilePath Win32StorageDirectoryDescriptor::ApplicationDataDirectory(const char* appname) {
	return Win32AppDataRoot + appname;
}

void Win32StorageDirectoryDescriptor::InitializePlatformRootDirectory(int argc, char* argv[]) {
	PWSTR path;
	HRESULT res = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
	ASSERT(res == S_OK) << res;
	Win32AppDataRoot = FilePath { path };
	StorageDirectoryDescriptor::initializeRootDirectory(ApplicationDataDirectory(APPLICATION_DEFAULT_NAME));
	StorageDirectoryDescriptor { StorageDirectoryDescriptor::Root() }.create();

	CoTaskMemFree(path);

	unsigned int len;
	char rootpath[MAX_PATH + 1];
	len = GetModuleFileName(NULL, rootpath, MAX_PATH);
	ASSERT(len != 0) << "Failed to get module file name";
	ASSERT(len < MAX_PATH + 1) << "Buffer might be insufficent when retrieving module file name";
	if (len == 0) {
		if (argc >= 1) {
			len = strlen(argv[0]);
			memcpy(rootpath, argv[0], len + 1);
		} else {
			//no modulefilename, no first argument?
			STORAGE_ASSETS_ROOT_DIR = ".";
			return;
		}
	}
	while (len > 0 && rootpath[len - 1] != FILE_SEPARATOR_CHAR) {
		--len;
	}
	if (len > 0) {
		rootpath[len - 1] = 0;
		STORAGE_ASSETS_ROOT_DIR = rootpath;
	} else {
		STORAGE_ASSETS_ROOT_DIR = FilePath { ".", 1 };
	}

	LOGI()<< "Win32 storage directory path: " << StorageDirectoryDescriptor::Root().getURI();
	LOGI()<< "Win32 asset directory path: " << STORAGE_ASSETS_ROOT_DIR.getURI();
}

Win32StorageDirectoryDescriptor::Win32StorageDirectoryDescriptor(FilePath path)
		: path { util::move(path) } {
}

bool Win32StorageDirectoryDescriptor::create() {
	BOOL success = CreateDirectoryW(path.getURI(), nullptr);
	ASSERT(success || GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "directory creation failed, last error: " << GetLastError();
	return success || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool Win32StorageDirectoryDescriptor::remove() {
	BOOL success = RemoveDirectoryW(path.getURI());
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "Failed to remove directory, last error: " << GetLastError();
	return true;
}

bool Win32StorageDirectoryDescriptor::exists() {
	WIN32_FILE_ATTRIBUTE_DATA data;
	BOOL success = GetFileAttributesExW(path.getURI(), GetFileExInfoStandard, &data);
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "Failed to query file attributes, last error: " << GetLastError();
	return success && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

class Win32FilePathIterator: public DirectoryDescriptor::FilePathIterator {
	static bool isSkipFile(const FilePathChar* path) {
		//skip paths '.', '..'
		return path[0] == '.' && (path[1] == 0 || (path[1] == '.' && path[2] == 0));
	}

	void goNextFile() {
		do {
			BOOL result = FindNextFileW(handle, &data);
			ASSERT(result || GetLastError() == ERROR_NO_MORE_FILES) << "Invalid error for FindNextFile: " << GetLastError();
			if (!result) {
				FindClose(handle);
				handle = INVALID_HANDLE_VALUE;
			}
		} while (handle != INVALID_HANDLE_VALUE && isSkipFile(data.cFileName));
	}

	HANDLE handle;
	WIN32_FIND_DATAW data;
public:
	Win32FilePathIterator()
			: handle(INVALID_HANDLE_VALUE) {
	}
	Win32FilePathIterator(const FilePath& path) {
		handle = FindFirstFileExW((path + "*").getURI(), FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, 0);
		WARN(handle == INVALID_HANDLE_VALUE) << "Failed to find first file, error: " << GetLastError() << " For file path: " << path.getURI();
		if (handle != INVALID_HANDLE_VALUE) {
			if (isSkipFile(data.cFileName)) {
				goNextFile();
			}
		}
	}
	virtual ~Win32FilePathIterator() {
		if (handle != INVALID_HANDLE_VALUE) {
			FindClose(handle);
		}
	}
	virtual bool operator !=(const DirectoryDescriptor::FilePathIterator& o) const override {
		const Win32FilePathIterator& it = static_cast<const Win32FilePathIterator&>(o);
		return handle != it.handle;
	}
	virtual Win32FilePathIterator& operator++() override {
		goNextFile();
		return *this;
	}

	virtual const FilePathChar* operator*() const override {
		return data.cFileName;
	}
	virtual bool isDirectory() const override {
		return HAS_FLAG(data.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	}
};

class Win32FilePathEnumerator: public DirectoryDescriptor::FilePathEnumerator {
	friend class Win32FilePathIterator;

	FilePath path;
public:
	Win32FilePathEnumerator(const FilePath& path)
			: path { path } {
	}
	Win32FilePathEnumerator(Win32FilePathEnumerator&& o)
			: path(util::move(o.path)) {
	}
	~Win32FilePathEnumerator() {
	}
	virtual DirectoryDescriptor::DirectoryIterator begin() override {
		return {new Win32FilePathIterator {path}};
	}
	virtual DirectoryDescriptor::DirectoryIterator end() override {
		return {new Win32FilePathIterator {}};
	}
};

DirectoryDescriptor::FilePathEnumerator * Win32StorageDirectoryDescriptor::createNewEnumerator() {
	return new Win32FilePathEnumerator { path };
}

} // namespace rhfw

