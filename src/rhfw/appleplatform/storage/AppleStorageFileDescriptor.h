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
 * AppleFileDescriptor.h
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#ifndef APPLEFILEDESCRIPTOR_H_
#define APPLEFILEDESCRIPTOR_H_

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <appleplatform/storage/AppleStorageFileInput.h>
#include <appleplatform/storage/AppleStorageFileOutput.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

class AppleStorageFileDescriptor final: public StorageFileDescriptorBase {
private:
	FilePath path;
public:
	AppleStorageFileDescriptor(FilePath path);
	AppleStorageFileDescriptor(AppleStorageFileDescriptor&& o) = default;
	virtual ~AppleStorageFileDescriptor();

	AppleStorageFileOutput* createOutput() override {
		return new AppleStorageFileOutput { path };
	}
	AppleStorageFileInput* createInput() override {
		return new AppleStorageFileInput { path };
	}

	bool exists() override;

	const FilePath& getPath() const {
		return path;
	}
	virtual bool create() override;
	virtual bool remove() override;

	virtual bool move(const StorageFileDescriptor& target) override;

	virtual long long size() override;
};

}

#endif /* APPLEFILEDESCRIPTOR_H_ */
