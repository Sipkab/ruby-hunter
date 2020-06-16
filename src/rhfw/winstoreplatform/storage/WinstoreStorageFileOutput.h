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
#ifndef WINSTORESTORAGEFILEOUTPUT_H_
#define WINSTORESTORAGEFILEOUTPUT_H_

#include <framework/io/files/FileOutput.h>
#include <framework/io/files/FilePath.h>

#include <Windows.h>

#include <gen/configuration.h>

namespace rhfw {

class WinstoreStorageFileOutput final : public FileOutput {
private:
	const FilePath& path;
	HANDLE file;

	virtual int writeImpl(const void* data, int count) override {
		DWORD res;
		BOOL success = WriteFile(file, data, count, &res, nullptr);
		ASSERT(success) << "Failed to write to file, error: " << GetLastError();
		return (int) res; //TODO long return value?
	}

	virtual bool openImpl() override {
		if (isAppend()) {
			file = CreateFile2(path.getURI(), GENERIC_WRITE, 0, OPEN_ALWAYS, nullptr);
			if (file != INVALID_HANDLE_VALUE) {
				BOOL success = SetFilePointerEx(file, { 0 }, nullptr, FILE_END);
				ASSERT(success) << "Failed to move file pointer to append, error: " << GetLastError();
			}
		} else {
			file = CreateFile2(path.getURI(), GENERIC_WRITE, 0, CREATE_ALWAYS, nullptr);
		}
		WARN(file == INVALID_HANDLE_VALUE) << "Failed to open file, " << path.getURI() << " last error: " << GetLastError();
		return file != INVALID_HANDLE_VALUE;
	}
	virtual void closeImpl() override {
		CloseHandle(file);
	}

	virtual bool flushDiskImpl() override {
		BOOL success = FlushFileBuffers(file);
		ASSERT(success) << "Failed to flush file buffer, error: " << GetLastError();
		return success != 0;
	}
public:
	WinstoreStorageFileOutput(const FilePath& path)
			: path(path), file { INVALID_HANDLE_VALUE } {
	}
	WinstoreStorageFileOutput(WinstoreStorageFileOutput&& o)
			: path(o.path), file { o.file } {
		o.file = INVALID_HANDLE_VALUE;
	}
	virtual ~WinstoreStorageFileOutput() {
		close();
	}

};

}

#endif /*WINSTORESTORAGEFILEOUTPUT_H_*/
