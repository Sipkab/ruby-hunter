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
 * FileOutput.cpp
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#include <framework/io/files/FileOutput.h>
#include <cstring>

#include <gen/log.h>

namespace rhfw {

FileOutput::~FileOutput() {
	ASSERT(!opened || closed) << "FileOutput was not closed";
}

bool FileOutput::open() {
	ASSERT(closed) << "File output is not closed";

	if (closed) {
		if (openImpl()) {
			if (bufferSize != 0) {
				buffer = new char[bufferSize];
				bufferCount = 0;
			}
			error = false;
			closed = false;
		} else {
			error = true;
			closed = true;
		}
		opened = true;
	}
	return !closed;
}
void FileOutput::close() {
	if (opened && !closed) {
		flushBuffer();
		closeImpl();

		closed = true;
		opened = false;
		error = false;

		delete[] buffer;
		buffer = nullptr;
	}
}

bool FileOutput::create() {
	ASSERT(!opened) << "File output is already opened";

	bool result = openImpl();
	if (result)
		closeImpl();
	return result;
}

bool FileOutput::writeCount(const void* data, unsigned int count) {
	int result = 0;
	while (count > 0 && (result = writeImpl(data, count)) >= 0) {
		WARN(result == 0) << "Write count returned 0, zero bytes were written.";
		data = static_cast<const char*>(data) + result;
		count -= result;
	}
	if (result < 0)
		return true;
	return false;
}

bool FileOutput::write(const void* data, int count) {
	ASSERT(opened && !closed) << "File is not open or is closed";
	if (error) {
		return false;
	}
	if (buffer != nullptr) {
		while (bufferCount + count >= bufferSize) {
			const int tocopy = bufferSize - bufferCount;
			memcpy(buffer + bufferCount, data, tocopy);
			error |= writeCount(buffer, bufferSize);
			if (error)
				return false;
			data = static_cast<const char*>(data) + tocopy;
			count -= tocopy;
			bufferCount = 0;
		}
		//enough size in buffer to store
		memcpy(buffer + bufferCount, data, count);
		bufferCount += count;
	} else {
		error |= writeCount(data, count);
		if (error) {
			return false;
		}
	}
	return true;
}

int FileOutput::flushBuffer() {
	ASSERT(opened && !closed) << "File is not open or is closed";
	if (error)
		return -1;
	if (buffer == nullptr || bufferCount == 0)
		return 0;

	const int towrite = bufferCount;
	error |= writeCount(buffer, towrite);
	bufferCount = 0;

	return error ? -1 : towrite;
}

void FileOutput::flushDisk() {
	ASSERT(opened && !closed) << "File is not open or is closed";
	error |= !flushDiskImpl();
}

FileOutput::FileOutput(FileOutput&& o)
		: buffer { o.buffer }, bufferSize { o.bufferSize }, bufferCount { o.bufferCount }, error { o.error }, closed { o.closed }, opened {
				o.opened }, append { o.append } {
	o.closed = true;
	o.buffer = nullptr;
}

FileOutput& FileOutput::operator =(FileOutput&& o) {
	ASSERT(this != &o) << "self move assignment";

	this->close();

	this->buffer = o.buffer;
	this->bufferSize = o.bufferSize;
	this->bufferCount = o.bufferCount;
	this->error = o.error;
	this->closed = o.closed;
	this->opened = o.opened;
	this->append = o.append;

	o.buffer = nullptr;
	o.closed = true;
	return *this;
}
}

