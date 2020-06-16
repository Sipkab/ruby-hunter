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
 * Win32StorageFileDescriptor.cpp
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/files/FileOutput.h>
#include <framework/utils/utility.h>

#include <Windows.h>

#include <gen/log.h>

namespace rhfw {

Win32StorageFileDescriptor::Win32StorageFileDescriptor(FilePath path)
		: path(util::move(path)) {
}

bool Win32StorageFileDescriptor::exists() {
	WIN32_FILE_ATTRIBUTE_DATA data;
	BOOL success = GetFileAttributesExW(path.getURI(), GetFileExInfoStandard, &data);
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
			<< "Failed to query file attributes, last error: " << GetLastError();
	return success != 0; // && (data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL;
}
bool Win32StorageFileDescriptor::create() {
	return Win32StorageFileOutput { path }.create();
}
bool Win32StorageFileDescriptor::remove() {
	LOGV()<< "Delete file or directory at: " << path.getURI();
	BOOL success = DeleteFileW(path.getURI());
	ASSERT(success || GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)
	<< "Failed to remove directory, last error: " << GetLastError();
	return true; //if failed to remove, then its not there
}

bool Win32StorageFileDescriptor::move(const StorageFileDescriptor& target) {
	BOOL res = MoveFileW(path.getURI(), target.path.getURI());
	WARN(!res) << "Failed to move file: " << path.getURI() << " to: " << target.path.getURI() << " error: " << GetLastError();
	return res;
}

#define WINDOWS_TICK 10000
#define MILLISEC_TO_UNIX_EPOCH 11644473600000LL

long long Win32StorageFileDescriptor::lastModified() {
	FILETIME outtime;
	HANDLE handle = CreateFileW(path.getURI(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		LOGW()<< "Failed to get open file: " << path.getURI()<< " " << GetLastError();
		return -1;
	}
	BOOL success = GetFileTime(handle, NULL, NULL, &outtime);
	CloseHandle(handle);
	ASSERT(success) << "Failed to get file time " << path.getURI() << " " << GetLastError();
	long long result = (long long) outtime.dwLowDateTime | ((long long) outtime.dwHighDateTime << 32);
	//result is in 100-nanoseconds

	return result / WINDOWS_TICK - MILLISEC_TO_UNIX_EPOCH;
}

long long Win32StorageFileDescriptor::size() {
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesExW(path.getURI(), GetFileExInfoStandard, &fad)) {
		LOGW()<< "Failed to get file attributres: " << path.getURI() << " " << GetLastError();
		return -1; // error condition, could call GetLastError to find out more
	}
	LARGE_INTEGER size;
	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;
	return size.QuadPart;
}

} // namespace rhfw

