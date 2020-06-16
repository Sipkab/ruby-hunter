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
 * Win32AssetFileInput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef WIN32ASSETFILEINPUT_H_
#define WIN32ASSETFILEINPUT_H_

#include <framework/io/files/FileInput.h>
#include <framework/utils/utility.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>

#include <win32platform/minwindows.h>

#include <string>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class Win32AssetFileInput final : public FileInput {
private:
	RAssetFile assetId;
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

	/**
	 * Return the bytes read.
	 * 0 on EOF, negative on ERROR, else the bytecount read.
	 */
	virtual int readImpl(void* buffer, unsigned int length) override {
		DWORD res;
		BOOL success = ReadFile(file, buffer, length, &res, nullptr);
		ASSERT(success) << "Failed to read from file, error: " << GetLastError();
		return (int)res; //TODO long return value?
	}

	virtual bool openImpl() override {
		ASSERT(assetId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "Invalid file to open";
		wchar_t pathbuf[11 + 8 + 1];
		swprintf_s(pathbuf, L"assets\\res\\%x", (unsigned int)assetId);

		FilePath path = StorageDirectoryDescriptor::AssetsRoot() + pathbuf;

		file = CreateFileW(path.getURI(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		WARN(file == INVALID_HANDLE_VALUE) << "Failed to open file, last error: " << GetLastError() << " file: " << path.getURI();
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
	Win32AssetFileInput(RAssetFile assetId)
		: assetId {assetId}, file {INVALID_HANDLE_VALUE} {
	}
	Win32AssetFileInput(Win32AssetFileInput&& o)
		: FileInput(util::move(o)), assetId {o.assetId}, file {o.file} {
		o.assetId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = nullptr;
	}
	Win32AssetFileInput& operator=(Win32AssetFileInput&& o) {
		FileInput::operator=(util::move(o));

		this->assetId = o.assetId;
		this->file = o.file;

		o.assetId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = INVALID_HANDLE_VALUE;

		return *this;
	}
	virtual ~Win32AssetFileInput() {
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

#endif /* WIN32ASSETFILEINPUT_H_ */
