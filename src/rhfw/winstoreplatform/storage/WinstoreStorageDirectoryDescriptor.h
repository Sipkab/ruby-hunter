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
 * WinstoreStorageDirectoryDescriptor.h
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#ifndef WINSTORESTORAGEDIRECTORYDESCRIPTOR_H_
#define WINSTORESTORAGEDIRECTORYDESCRIPTOR_H_

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <gen/configuration.h>

namespace rhfw {

class WinstoreStorageDirectoryDescriptor final : public StorageDirectoryDescriptorBase {
	FilePath path;

	virtual FilePathEnumerator* createNewEnumerator() override;
public:
	static void InitializePlatformRootDirectory();

	WinstoreStorageDirectoryDescriptor(FilePath path);
	~WinstoreStorageDirectoryDescriptor() {
	}

	virtual bool create() override;
	virtual bool remove() override;

	virtual bool exists() override;

	const FilePath& getPath() const {
		return path;
	}
};

}
#endif /* WINSTORESTORAGEDIRECTORYDESCRIPTOR_H_ */
