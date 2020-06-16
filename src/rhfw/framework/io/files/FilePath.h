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
 * FilePath.h
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#ifndef FILEPATH_H_
#define FILEPATH_H_

#include <framework/utils/utility.h>

#include <cstring>

#include <gen/configuration.h>
#include <gen/platform.h>
#include <gen/log.h>

namespace rhfw {

class FilePath {
	FilePathChar* path;
	unsigned int length;

	template<typename Chartype>
	static unsigned int _pstrlen(const Chartype* path) {
		ASSERT(path != nullptr) << "Path is nullptr";
		const Chartype* p = path;
		while (*p)
			++p;
		return p - path;
	}
public:

	FilePath()
			: path { nullptr }, length { 0 } {
	}
	template<typename Chartype>
	FilePath(const Chartype* p, unsigned int length)
			: path { new FilePathChar[length + 1] }, length { length } {
		ASSERT(p != nullptr) << "Path is nullptr";
		for (unsigned int i = 0; i < length; ++i) {
			path[i] = static_cast<FilePathChar>(p[i]);
		}
		path[length] = 0;
	}
	/*template<typename Chartype, unsigned int n>
	 FilePath(const Chartype (&path)[n])
	 : FilePath { path, n - 1 } {
	 }*/

	template<typename Chartype>
	FilePath(const Chartype* path)
			: FilePath { path, _pstrlen(path) } {
	}
	FilePath(FilePath&& o)
			: path { o.path }, length { o.length } {
		o.path = nullptr;
	}
	FilePath(const FilePath& o)
			: FilePath { o.path, o.length } {
	}
	FilePath& operator=(FilePath o);

	~FilePath() {
		delete[] path;
	}

	void swap(FilePath& o) {
		util::swap(this->length, o.length);
		util::swap(this->path, o.path);
	}
	template<typename Chartype>
	FilePath append(const Chartype* p, unsigned int length) const {
		ASSERT(this->path != nullptr && p != nullptr) << "Path is nullptr, assign to it first";
		FilePath res { };
		res.length = this->length + length + 1;
		res.path = new FilePathChar[res.length + 1];

		memcpy(res.path, this->path, sizeof(FilePathChar) * this->length);
		res.path[this->length] = static_cast<FilePathChar>(FILE_SEPARATOR_CHAR);

		for (unsigned int i = 0; i <= length; ++i) {
			res.path[this->length + 1 + i] = static_cast<FilePathChar>(p[i]);
		}
		return util::move(res);
	}

	FilePath operator+(const FilePath& o) const {
		return append(o.path, o.length);
	}
	/*template<typename Chartype, unsigned int n>
	 FilePath operator+(const Chartype (&p)[n]) const {
	 return append(p, n - 1);
	 }*/

	FilePath& operator+=(const FilePath& o) {
		return *this = *this + o;
	}
	/*template<typename Chartype, unsigned int n>
	 FilePath& operator+=(const Chartype (&p)[n]) {
	 return *this = *this + p;
	 }*/

	template<typename Chartype>
	FilePath operator+(const Chartype* p) const {
		return append(p, _pstrlen(p));
	}

	const FilePathChar* getURI() const {
		return path;
	}
	unsigned int getLength() const {
		return length;
	}

	template<typename Chartype>
	bool startsWith(const Chartype* p) const {
		//TODO
		unsigned int i = 0;
		for (; i < this->length && static_cast<FilePathChar>(p[i]) == this->path[i]; ++i)
			;
		return i == this->length;
	}

	bool startsWith(const FilePath& path) const {
		return startsWith(path.getURI());
	}
};
namespace util {
template<>
inline void swap(FilePath & a, FilePath & b) {
	a.swap(b);
}
}

inline FilePath& FilePath::operator =(FilePath o) {
	util::swap(*this, o);
	return *this;
}

}
#endif /* FILEPATH_H_ */
