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
#ifndef WINSTORESTORAGEFILEINPUT_H_
#define WINSTORESTORAGEFILEINPUT_H_

#include <framework/io/files/FileInput.h>
#include <framework/io/files/FilePath.h>

#include <Windows.h>
#include <gen/types.h>

#include <gen/configuration.h>

namespace rhfw {

class WinstoreStorageFileInput final : public FileInput {
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
		file = CreateFile2(path.getURI(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
		WARN(file == INVALID_HANDLE_VALUE) << "Failed to open file, last error: " << GetLastError();
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
	WinstoreStorageFileInput(const FilePath& path)
		: path(path) {
	}
	WinstoreStorageFileInput(WinstoreStorageFileInput&& o) : path(o.path), file {o.file} {
		o.file = INVALID_HANDLE_VALUE;
	}
	virtual ~WinstoreStorageFileInput() {
		close();
	}

	virtual long long size() override {
		LARGE_INTEGER li;
		BOOL res = GetFileSizeEx(file, &li);
		ASSERT(res) << "Failed to get file size";
		if (!res) {
			return 0;
		}
		return li.QuadPart;
	}
};

}

#endif /*WINSTORESTORAGEFILEINPUT_H_*/
