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
 * FileDescriptor.h
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#ifndef FILEDESCRIPTOR_H_
#define FILEDESCRIPTOR_H_

#include <framework/io/stream/OwnerStream.h>
#include <framework/io/files/FileInput.h>
#include <framework/io/files/FileOutput.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class FileOutput;
class FileInput;

class FileDescriptor {
public:
	FileDescriptor() = default;
	FileDescriptor(FileDescriptor&&) = default;
	FileDescriptor(const FileDescriptor&) = default;
	FileDescriptor& operator=(FileDescriptor&&) = default;
	FileDescriptor& operator=(const FileDescriptor&) = default;
	virtual ~FileDescriptor() = default;

	virtual FileOutput* createOutput() = 0;
	virtual FileInput* createInput() = 0;

	//using trailing return type crashes msvc...
	OwnerInputStream::Wrap<FileInput> openInputStream() {
		auto* file = createInput();
		file->open();
		return OwnerInputStream::wrap(file);
	}
	OwnerOutputStream::Wrap<FileOutput> openOutputStream() {
		auto* file = createOutput();
		file->open();
		return OwnerOutputStream::wrap(file);
	}
	OwnerOutputStream::Wrap<FileOutput> openAppendStream() {
		auto* file = createOutput();
		file->setAppend(true);
		file->open();
		return OwnerOutputStream::wrap(file);
	}

	virtual char* readFully(unsigned int* len) {
		FileInput* in = createInput();
		if (!in->open()) {
			*len = 0;
			return nullptr;
		}
		long long size = in->size();
		ASSERT(size >= 0) << size;

		if (size < 0) {
			return nullptr;
		}
		char* buffer = new char[(unsigned int) size];
		int read = in->read(buffer, size);
		delete in;

		ASSERT(read == size) << read << " - " << size;
		if (read < 0) {
			*len = 0;
			delete[] buffer;
			buffer = nullptr;
		} else {
			//read was successful, here we dont return nullptr, to indicate non-failure
			*len = (unsigned int) read;
		}
		return buffer;
	}

	/**
	 * Returns true if the file referenced by this descriptor exists.
	 */
	virtual bool exists() = 0;
	/**
	 * Returns true if the file referenced by this descriptor represents a directory.
	 */
	virtual bool isDirectory() = 0;
	/**
	 * Returns true if the file referenced by this descriptor represents a file.
	 */
	virtual bool isFile() = 0;

	/**
	 * Returns true if the file or directory exists after this function call. If it already existed, then it is not modified.
	 */
	virtual bool create() = 0;
	/**
	 * Returns true if the file doesn't exist after this function call.
	 */
	virtual bool remove() = 0;

	/**
	 * Returns the time when the file was last modified. The result is the number of milliseconds elapsed since the UNIX epoch in UTC (1970 January 1).
	 * In case of failure, -1 is returned;
	 */
	virtual long long lastModified() {
		return -1;
	}

	/**
	 * Returns the size of the file, -1 on error.
	 */
	virtual long long size() {
		return -1;
	}
};

} // namespace rhfw

#endif /* FILEDESCRIPTOR_H_ */
