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
 * StorageFileDescriptor.h
 *
 *  Created on: 2015 okt. 26
 *      Author: sipka
 */

#ifndef STORAGEDIRECTORYDESCRIPTOR_H_
#define STORAGEDIRECTORYDESCRIPTOR_H_

#include <framework/io/files/DirectoryDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <gen/platform.h>

namespace rhfw {

class StorageDirectoryDescriptorBase: public DirectoryDescriptor {
	static FilePath STORAGE_ROOT_DIR;
public:
	static void initializeRootDirectory(FilePath path);
	static const FilePath& Root() {
		return STORAGE_ROOT_DIR;
	}
};

} // namespace rhfw

#include STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class STORAGEDIRECTORYDESCRIPTOR_EXACT_CLASS_TYPE StorageDirectoryDescriptor;
} // namespace rhfw

#endif /* STORAGEDIRECTORYDESCRIPTOR_H_ */
