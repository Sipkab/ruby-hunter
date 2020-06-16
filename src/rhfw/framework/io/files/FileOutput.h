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
 * FileOutput.h
 *
 *  Created on: 2015 febr. 28
 *      Author: sipka
 */

#ifndef FILEOUTPUT_H_
#define FILEOUTPUT_H_

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class FileOutput {
private:
	//buffer containing the data to write out in batch
	char* buffer = nullptr;
	//the allocated buffer size
	unsigned int bufferSize = 1024;
	//the data count in buffer
	unsigned int bufferCount = 0;

	//if error happened on IO
	bool error = false;
	//is file closed
	bool closed = true;
	//is file opened
	bool opened = false;

protected:
	//whether should append to the opened file
	bool append = false;

private:
	virtual bool openImpl() = 0;
	virtual void closeImpl() = 0;

	virtual int writeImpl(const void* data, int count) = 0;

	virtual bool flushDiskImpl() = 0;

	//return true if error happened
	bool writeCount(const void* data, unsigned int count);
public:
	FileOutput() {
	}
	virtual ~FileOutput();
	FileOutput(const FileOutput&) = delete;
	FileOutput& operator=(const FileOutput&) = delete;
	FileOutput(FileOutput&& o);
	FileOutput& operator=(FileOutput&& o);

	bool open();
	void close();
	bool create();

	/**
	 * Returns true if write succeeded
	 */
	bool write(const void* data, int count);

	template<typename T>
	bool write(const T& data) {
		return write(&data, sizeof(T));
	}

	void setBufferSize(unsigned int size) {
		ASSERT(!opened) << "FileOutput already opened";
		this->bufferSize = size;
	}

	int flushBuffer();
	void flushDisk();

	bool isError() const {
		return error;
	}
	bool isClosed() const {
		return closed;
	}
	bool isOpened() const {
		return opened;
	}
	bool canOpen() const {
		return !opened;
	}

	bool isAppend() const {
		return append;
	}
	void setAppend(bool append) {
		ASSERT(!opened) << "FileOutput already opened";
		this->append = append;
	}

};
}

#endif /* FILEOUTPUT_H_ */
