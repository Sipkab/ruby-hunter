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
 * WinstoreAssetFileInput.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef WINSTOREASSETFILEINPUT_H_
#define WINSTOREASSETFILEINPUT_H_

#include <framework/io/files/FileInput.h>
#include <framework/utils/utility.h>

#include <Windows.h>

#include <string>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class WinstoreAssetFileInput final : public FileInput {
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
		//TODO sometimes it crashes here with ERROR_NOACCESS (998)
		ASSERT(success) << "Failed to read from file, error: " << GetLastError();
		return (int)res;//TODO long return value?
	}

	virtual bool openImpl() override {
		ASSERT(assetId != RAssetFile::INVALID_ASSET_IDENTIFIER) << "Invalid file to open";
		//auto localFolderPath = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
		//auto filePath = "assets\\res\\" + resourceId;// (localFolderPath + "\\assets\\res\\") + resourceId;
		wchar_t pathbuf[16];
		swprintf_s(pathbuf, L"%x", (unsigned int)assetId);

		file = CreateFile2((L"assets\\res\\" + std::wstring(pathbuf)).c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
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
	WinstoreAssetFileInput(RAssetFile assetId)
		: assetId {assetId}, file {INVALID_HANDLE_VALUE} {
	}
	WinstoreAssetFileInput(WinstoreAssetFileInput&& o)
		: FileInput(util::move(o)), assetId {o.assetId}, file {o.file} {
		o.assetId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = nullptr;
	}
	WinstoreAssetFileInput& operator=(WinstoreAssetFileInput&& o) {
		FileInput::operator=(util::move(o));

		this->assetId = o.assetId;
		this->file = o.file;

		o.assetId = RAssetFile::INVALID_ASSET_IDENTIFIER;
		o.file = INVALID_HANDLE_VALUE;

		return *this;
	}
	virtual ~WinstoreAssetFileInput() {
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

#endif /* WINSTOREASSETFILEINPUT_H_ */
