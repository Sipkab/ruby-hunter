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

#ifndef STORAGEFILEDESCRIPTOR_H_
#define STORAGEFILEDESCRIPTOR_H_

#include <framework/io/files/FileDescriptor.h>
#include <framework/utils/InputSource.h>

#include <gen/configuration.h>
#include <gen/platform.h>

namespace rhfw {

typedef class STORAGEFILEDESCRIPTOR_EXACT_CLASS_TYPE StorageFileDescriptor;

class StorageFileDescriptorBase: public FileDescriptor {
public:
	bool isDirectory() override final {
		return false;
	}
	bool isFile() override final {
		return true;
	}

	virtual bool move(const StorageFileDescriptor& target) = 0;
};

}  // namespace rhfw

#include STORAGEFILEDESCRIPTOR_EXACT_CLASS_INCLUDE

namespace rhfw {

class StorageInputSource: public InputSource {
private:
	StorageFileDescriptor storagefd;

	virtual void closeData(InputSource::Data& data) override {
		delete[] static_cast<const char*>(data);
	}
public:
	StorageInputSource(StorageFileDescriptor sfd)
			: storagefd { util::move(sfd) } {
	}
	virtual InputSource::Data getData() override {
		unsigned int len;
		const char* data = storagefd.readFully(&len);
		return {this, data, len};
	}

};

}  // namespace rhfw

#endif /* STORAGEFILEDESCRIPTOR_H_ */
