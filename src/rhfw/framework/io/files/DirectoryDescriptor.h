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

#ifndef DIRECTORYDESCRIPTOR_H_
#define DIRECTORYDESCRIPTOR_H_

#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/FilePath.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class DirectoryDescriptor: public FileDescriptor {
public:

	class FilePathIterator {
	public:
		virtual bool operator !=(const FilePathIterator& o) const = 0;
		virtual FilePathIterator& operator++() = 0;
		virtual const FilePathChar* operator*() const = 0;
		virtual bool isDirectory() const = 0;
		virtual ~FilePathIterator() = default;
	};
	class DirectoryEntry {
		FilePathIterator& iterator;
	public:
		DirectoryEntry(FilePathIterator& iterator)
				: iterator(iterator) {
		}
		bool isDirectory() const {
			return iterator.isDirectory();
		}
		operator const FilePathChar*() const {
			return *iterator;
		}
		operator FilePath() const {
			return FilePath { *iterator };
		}
	};
	class DirectoryIterator {
		FilePathIterator* iterator;
	public:
		DirectoryIterator(FilePathIterator* iterator)
				: iterator { iterator } {
		}
		DirectoryIterator(DirectoryIterator&& o)
				: iterator { o.iterator } {
			o.iterator = nullptr;
		}
		~DirectoryIterator() {
			delete iterator;
		}
		bool operator !=(const DirectoryIterator& o) const {
			return *iterator != *o.iterator;
		}
		DirectoryIterator& operator++() {
			++(*iterator);
			return *this;
		}
		DirectoryEntry operator*() {
			return {*iterator};
		}
	};
	class FilePathEnumerator {
	public:
		virtual DirectoryIterator begin() = 0;
		virtual DirectoryIterator end() = 0;
		virtual ~FilePathEnumerator() {
		}
	};
	class DirectoryEnumerator {
		FilePathEnumerator* enumerator;
	public:
		DirectoryEnumerator(FilePathEnumerator* enumerator)
				: enumerator { enumerator } {
		}
		DirectoryEnumerator(DirectoryEnumerator&& o)
				: enumerator { o.enumerator } {
			o.enumerator = nullptr;
		}
		~DirectoryEnumerator() {
			delete enumerator;
		}

		DirectoryIterator begin() {
			return enumerator->begin();
		}
		DirectoryIterator end() {
			return enumerator->end();
		}
	};
private:
	virtual FilePathEnumerator* createNewEnumerator() = 0;
public:
	FileOutput* createOutput() override final {
		THROW()<<"Directories doesn't have output";
		return nullptr;
	}
	FileInput* createInput() override final {
		THROW() << "Directories doesn't have input";
		return nullptr;
	}

	bool isDirectory() override final {
		return true;
	}
	bool isFile() override final {
		return false;
	}

	DirectoryEnumerator enumerate() {
		return DirectoryEnumerator {createNewEnumerator()};
	}
};
}

#endif /* DIRECTORYDESCRIPTOR_H_ */
