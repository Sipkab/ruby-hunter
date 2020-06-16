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
 * Win32StorageDirectoryDescriptor.h
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#ifndef WIN32STORAGEDIRECTORYDESCRIPTOR_H_
#define WIN32STORAGEDIRECTORYDESCRIPTOR_H_

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <gen/configuration.h>

namespace rhfw {

class Win32StorageDirectoryDescriptor final : public StorageDirectoryDescriptorBase {
	FilePath path;

	virtual FilePathEnumerator* createNewEnumerator() override;

	static FilePath STORAGE_ASSETS_ROOT_DIR;
public:

	static const FilePath& ApplicationDataRoot();
	static FilePath ApplicationDataDirectory(const char* appname);

	static void InitializePlatformRootDirectory(int argc, char* argv[]);

	static const FilePath& AssetsRoot() {
		return STORAGE_ASSETS_ROOT_DIR;
	}

	Win32StorageDirectoryDescriptor(FilePath path);
	~Win32StorageDirectoryDescriptor() {
	}

	virtual bool create() override;
	virtual bool remove() override;

	virtual bool exists() override;

	const FilePath& getPath() const {
		return path;
	}
};

}
#endif /* WIN32STORAGEDIRECTORYDESCRIPTOR_H_ */
