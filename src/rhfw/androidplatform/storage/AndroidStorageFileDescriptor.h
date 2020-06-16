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
 * AndroidFileDescriptor.h
 *
 *  Created on: 2015 aug. 16
 *      Author: sipka
 */

#ifndef ANDROIDFILEDESCRIPTOR_H_
#define ANDROIDFILEDESCRIPTOR_H_

#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <androidplatform/AndroidFile.h>
#include <androidplatform/storage/AndroidStorageFileInput.h>
#include <androidplatform/storage/AndroidStorageFileOutput.h>

#include <gen/types.h>
#include <gen/configuration.h>

namespace rhfw {

class AndroidStorageFileDescriptor final: public StorageFileDescriptorBase, public AndroidFile {
private:
	FilePath path;
public:
	AndroidStorageFileDescriptor(FilePath path);
	AndroidStorageFileDescriptor(AndroidStorageFileDescriptor&& o) = default;
	virtual ~AndroidStorageFileDescriptor();

	virtual AndroidStorageFileOutput* createOutput() override {
		return new AndroidStorageFileOutput { path };
	}
	virtual AndroidStorageFileInput* createInput() override {
		return new AndroidStorageFileInput { path };
	}

	virtual bool exists() override;

	const FilePath& getPath() const {
		return path;
	}
	virtual bool create() override;
	virtual bool remove() override;

	virtual bool move(const StorageFileDescriptor& target) override;

	//TODO lastmodified

	virtual AndroidFd openAndroidFd() override;

	//TODO
	virtual long long size() override;
};

}

#endif /* ANDROIDFILEDESCRIPTOR_H_ */
