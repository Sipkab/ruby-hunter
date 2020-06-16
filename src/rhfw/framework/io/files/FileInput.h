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
 * File.h
 *
 *  Created on: 2014.06.23.
 *      Author: sipka
 */

#ifndef FILE_H_
#define FILE_H_

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/fwd/types.h>

namespace rhfw {

class FileInput {
private:
	long long position = 0;

	//is file closed
	bool closed = true;
	//is file opened
	bool opened = false;

	/**
	 * Return the bytes read.
	 * 0 on EOF, negative on ERROR, else the bytecount read.
	 */
	virtual int readImpl(void* buffer, unsigned int count) = 0;
	virtual bool openImpl() = 0;
	virtual void closeImpl() = 0;
	virtual long long seekImpl(long long offset, SeekMethod method) = 0;

protected:
public:
	FileInput() {
	}
	FileInput(const FileInput&) = delete;
	FileInput& operator=(const FileInput&) = delete;
	FileInput(FileInput&& o);
	FileInput& operator=(FileInput&& o);
	virtual ~FileInput();

	/**
	 * Open the file
	 */
	bool open();
	/**
	 * Close the file
	 */
	void close();

	bool isClosed() const {
		return closed;
	}
	bool isOpened() const {
		return opened;
	}
	bool canOpen() const {
		return closed;
	}

	/**
	 * Return the bytes read.
	 * 0 on EOF, negative on ERROR, else the bytecount read.
	 */
	int read(void* buffer, unsigned int count);

	/**
	 * Seeks the file, returns the new position. On error, returns negative, and sets eof to true.
	 */
	long long seek(long long offset, SeekMethod method);
	long long getPosition() {
		return position;
	}

	long long skip(unsigned long long count);

	/**
	 * Returns the size of the file.
	 */
	virtual long long size() = 0;
};

}

#endif /* FILE_H_ */
