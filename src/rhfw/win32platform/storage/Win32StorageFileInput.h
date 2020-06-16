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
#ifndef WIN32STORAGEFILEINPUT_H_
#define WIN32STORAGEFILEINPUT_H_

#include <framework/io/files/FileInput.h>
#include <framework/io/files/FilePath.h>

#include <win32platform/minwindows.h>

#include <gen/configuration.h>
#include <gen/types.h>

namespace rhfw {

class Win32StorageFileInput final : public FileInput {
private:
	const FilePath& path;
	HANDLE file;

	//TODO remove duplicate here and asset file input
	static int convertSeekMethod(SeekMethod method) {
		switch (method) {
			case SeekMethod::BEGIN: {
				return FILE_BEGIN;
			}
			case SeekMethod::CURRENT: {
				return FILE_CURRENT;
			}
			case SeekMethod::END: {
				return FILE_END;
			}
			default: {
				THROW()<<"Invalid seek method " << method;
				return SEEK_SET;
			}
		}
	}

	virtual int readImpl(void* buffer, unsigned int length) override {
		DWORD res;
		BOOL success = ReadFile(file, buffer, length, &res, nullptr);
		ASSERT(success) << "Failed to read from file, error: " << GetLastError();
		return (int)res; //TODO long return value?
	}

	virtual bool openImpl() override {
		file = CreateFileW(path.getURI(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		//WARN(file == INVALID_HANDLE_VALUE) << "Failed to open file, last error: " << GetLastError() << " " << path.getURI();
		return file != INVALID_HANDLE_VALUE;
	}
	virtual void closeImpl() override {
		CloseHandle(file);
	}
	virtual long long seekImpl(long long offset, SeekMethod method) override {
		LARGE_INTEGER outoffset;
		LARGE_INTEGER target;
		target.QuadPart = offset;
		BOOL res = SetFilePointerEx(file, target, &outoffset, convertSeekMethod(method));
		ASSERT(res) << "SetFilePointer failed, error: " << GetLastError();
		return outoffset.QuadPart;
	}
public:
	Win32StorageFileInput(const FilePath& path)
	: path(path) {
	}
	Win32StorageFileInput(Win32StorageFileInput&& o)
	: FileInput(util::move(o)), path(o.path), file {o.file} {
		o.file = INVALID_HANDLE_VALUE;
	}
	virtual ~Win32StorageFileInput() {
		close();
	}

	virtual long long size() override {
		LARGE_INTEGER li;
		BOOL res = GetFileSizeEx(file, &li);
		ASSERT(res) << "Failed to get file size: " << GetLastError() << " " << path.getURI();
		if (!res) {
			return 0;
		}
		return li.QuadPart;
	}
};

}

#endif /*WIN32STORAGEFILEINPUT_H_*/
