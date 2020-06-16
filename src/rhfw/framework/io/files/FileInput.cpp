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
 * File.cpp
 *
 *  Created on: 2015 febr. 6
 *      Author: sipka
 */

#include <framework/io/files/FileInput.h>

#include <gen/log.h>
#include <gen/types.h>

namespace rhfw {

FileInput::FileInput(FileInput&& o)
		: position(o.position), closed(o.closed), opened(o.opened) {
	o.closed = true;
}
FileInput::~FileInput() {
	ASSERT(!opened || closed) << "FileInput was not closed";
}

FileInput& FileInput::operator =(FileInput&& o) {
	ASSERT(this != &o) << "self move assignment";

	this->close();

	this->closed = o.closed;
	this->opened = o.opened;
	this->position = o.position;

	o.closed = true;
	return *this;
}

bool FileInput::open() {
	ASSERT(closed) << "File input is not closed";

	if (closed) {
		if (openImpl()) {
			closed = false;
		} else {
			closed = true;
		}
		opened = true;
		position = 0;
	}
	return !closed;
}
void FileInput::close() {
	if (opened && !closed) {
		closeImpl();
		closed = true;
	}
}

long long FileInput::seek(long long offset, SeekMethod method) {
	long long seekres = seekImpl(offset, method);
	if (seekres >= 0) {
		this->position = seekres;
	}
	return seekres;
}
long long FileInput::skip(unsigned long long count) {
	return seek(count, SeekMethod::CURRENT);
}

int FileInput::read(void* buffer, unsigned int count) {
	if (!opened || closed) {
		//LOGW()<<"Trying to read from not opened file input";
		return -1;
	}
	int res = readImpl(buffer, count);
	if (res > 0) {
		position += res;
	}
	return res;
}

}
 // namespace rhfw

