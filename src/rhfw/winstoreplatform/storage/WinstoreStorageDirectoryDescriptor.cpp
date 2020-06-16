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
 * WinstoreStorageDirectoryDescriptor.cpp
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/utils/utility.h>

#include <Windows.h>

#include <gen/log.h>
#include <gen/types.h>

namespace rhfw {

void WinstoreStorageDirectoryDescriptor::InitializePlatformRootDirectory() {
	auto s = ::Windows::Storage::ApplicationData::Current->LocalFolder->Path;
	StorageDirectoryDescriptor::initializeRootDirectory(FilePath { s->Data(), s->Length() });
}

WinstoreStorageDirectoryDescriptor::WinstoreStorageDirectoryDescriptor(FilePath path)
		: path { util::move(path) } {
}

bool WinstoreStorageDirectoryDescriptor::create() {
	BOOL success = CreateDirectoryW(path.getURI(), nullptr);
	ASSERT(success || GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "directory creation failed, last error: " << GetLastError();
	return success || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool WinstoreStorageDirectoryDescriptor::remove() {
	BOOL success = RemoveDirectoryW(path.getURI());
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "Failed to remove directory, last error: " << GetLastError();
	return true;
}

bool WinstoreStorageDirectoryDescriptor::exists() {
	WIN32_FILE_ATTRIBUTE_DATA data;
	BOOL success = GetFileAttributesExW(path.getURI(), GetFileExInfoStandard, &data);
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "Failed to query file attributes, last error: " << GetLastError();
	return success && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

class WinstoreFilePathIterator: public DirectoryDescriptor::FilePathIterator {
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
	WinstoreFilePathIterator()
			: handle(INVALID_HANDLE_VALUE) {
	}
	WinstoreFilePathIterator(const FilePath& path) {
		handle = FindFirstFileExW((path + "*").getURI(), FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, 0);
		WARN(handle == INVALID_HANDLE_VALUE) << "Failed to find first file, error: " << GetLastError();
		if (isSkipFile(data.cFileName)) {
			goNextFile();
		}
	}
	virtual ~WinstoreFilePathIterator() {
		if (handle != INVALID_HANDLE_VALUE) {
			FindClose(handle);
		}
	}
	virtual bool operator !=(const DirectoryDescriptor::FilePathIterator& o) const override {
		const WinstoreFilePathIterator& it = static_cast<const WinstoreFilePathIterator&>(o);
		return handle != it.handle;
	}
	virtual WinstoreFilePathIterator& operator++() override {
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

class WinstoreFilePathEnumerator: public DirectoryDescriptor::FilePathEnumerator {
	friend class WinstoreFilePathIterator;

	FilePath path;
public:
	WinstoreFilePathEnumerator(const FilePath& path)
			: path { path } {
	}
	WinstoreFilePathEnumerator(WinstoreFilePathEnumerator&& o)
			: path(util::move(o.path)) {
	}
	~WinstoreFilePathEnumerator() {
	}
	virtual DirectoryDescriptor::DirectoryIterator begin() override {
		return {new WinstoreFilePathIterator {path}};
	}
	virtual DirectoryDescriptor::DirectoryIterator end() override {
		return {new WinstoreFilePathIterator {}};
	}
};

DirectoryDescriptor::FilePathEnumerator * WinstoreStorageDirectoryDescriptor::createNewEnumerator() {
	return new WinstoreFilePathEnumerator { path };
}

} // namespace rhfw

