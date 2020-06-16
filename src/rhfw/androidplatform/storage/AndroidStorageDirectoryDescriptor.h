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
 * AndroidStorageDirectoryDescriptor.h
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#ifndef ANDROIDSTORAGEDIRECTORYDESCRIPTOR_H_
#define ANDROIDSTORAGEDIRECTORYDESCRIPTOR_H_

#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class AndroidStorageDirectoryDescriptor final: public StorageDirectoryDescriptorBase {
	FilePath path;

	virtual FilePathEnumerator* createNewEnumerator() override;
public:

	AndroidStorageDirectoryDescriptor(FilePath path);
	~AndroidStorageDirectoryDescriptor() {
	}

	virtual bool create() override;
	virtual bool remove() override;

	virtual bool exists() override;

	const FilePath& getPath() const {
		return path;
	}
};

}
#endif /* ANDROIDSTORAGEDIRECTORYDESCRIPTOR_H_ */
