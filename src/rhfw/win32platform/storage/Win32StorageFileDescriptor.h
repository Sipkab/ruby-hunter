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
 * Win32FileDescriptor.h
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#ifndef WIN32FILEDESCRIPTOR_H_
#define WIN32FILEDESCRIPTOR_H_

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <win32platform/storage/Win32StorageFileInput.h>
#include <win32platform/storage/Win32StorageFileOutput.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

class Win32StorageFileDescriptor final : public StorageFileDescriptorBase {
	FilePath path;
public:
	Win32StorageFileDescriptor(FilePath path);
	Win32StorageFileDescriptor(Win32StorageFileDescriptor&& o) = default;
	~Win32StorageFileDescriptor() {
	}

	Win32StorageFileInput* createInput() override {
		return new Win32StorageFileInput { path };
	}
	Win32StorageFileOutput* createOutput() override {
		return new Win32StorageFileOutput { path };
	}

	WrapperInputStream<Win32StorageFileInput, InputStream> openInputStream() {
		Win32StorageFileInput fi { path };
		fi.open();
		return WrapperInputStream<Win32StorageFileInput, InputStream>::wrap(util::move(fi));
	}

	bool exists() override;

	const FilePath& getPath() const {
		return path;
	}

	virtual bool create() override;
	virtual bool remove() override;

	virtual bool move(const StorageFileDescriptor& target) override;

	virtual long long lastModified() override;

	virtual long long size() override;
};

}

#endif /* WIN32FILEDESCRIPTOR_H_ */
