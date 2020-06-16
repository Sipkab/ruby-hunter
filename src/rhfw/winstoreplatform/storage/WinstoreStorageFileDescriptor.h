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
 * WinstoreFileDescriptor.h
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#ifndef WINSTOREFILEDESCRIPTOR_H_
#define WINSTOREFILEDESCRIPTOR_H_

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <winstoreplatform/storage/WinstoreStorageFileInput.h>
#include <winstoreplatform/storage/WinstoreStorageFileOutput.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

class WinstoreStorageFileDescriptor final : public StorageFileDescriptorBase {
	FilePath path;
public:
	WinstoreStorageFileDescriptor(FilePath path);
	WinstoreStorageFileDescriptor(WinstoreStorageFileDescriptor&& o) = default;
	~WinstoreStorageFileDescriptor() {
	}

	WinstoreStorageFileInput* createInput() override {
		return new WinstoreStorageFileInput { path };
	}
	WinstoreStorageFileOutput* createOutput() override {
		return new WinstoreStorageFileOutput { path };
	}

	bool exists() override;

	const FilePath& getPath() const {
		return path;
	}

	virtual bool create() override;
	virtual bool remove() override;

	virtual bool move(const StorageFileDescriptor& target) override;

	virtual long long lastModified() override;

	//TODO
	virtual long long size() override;
};

}

#endif /* WINSTOREFILEDESCRIPTOR_H_ */
