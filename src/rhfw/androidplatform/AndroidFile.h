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
 * AndroidFile.h
 *
 *  Created on: 2016. marc. 3.
 *      Author: sipka
 */

#ifndef ANDROIDFILE_H_
#define ANDROIDFILE_H_

namespace rhfw {

class AndroidFd {
private:
public:
	int fd;
	long long start;
	long long length;
	AndroidFd()
			: fd { -1 }, start { 0 }, length { 0 } {
	}
	AndroidFd(int fd, long long start, long long length)
			: fd { fd }, start { start }, length { length } {
	}
};

class AndroidFile {
private:
public:
	AndroidFile() = default;
	AndroidFile(AndroidFile&&) = default;
	AndroidFile& operator=(AndroidFile&&) = default;
	virtual ~AndroidFile() = default;
	/**
	 * Just opens the file, and returns a file descriptor (if the descriptor is < 0, then it is invalid).
	 * Closing is the resposibility of the caller (calling close(int)).
	 */
	virtual AndroidFd openAndroidFd() = 0;
};

}  // namespace rhfw

#endif /* ANDROIDFILE_H_ */
