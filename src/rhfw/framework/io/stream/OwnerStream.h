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
 * OwnerStream.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_STREAM_OWNERSTREAM_H_
#define FRAMEWORK_IO_STREAM_OWNERSTREAM_H_

#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

namespace rhfw {

class OwnerInputStream: public InputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Arg>
	static Wrap<Arg> wrap(Arg* arg) {
		return Wrap<Arg>(arg);
	}
};

class OwnerOutputStream: public OutputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Arg>
	static Wrap<Arg> wrap(Arg* arg) {
		return Wrap<Arg>(arg);
	}
};
class OwnerInputOutputStream: public InputStream, public OutputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Arg>
	static Wrap<Arg> wrap(Arg* arg) {
		return Wrap<Arg>(arg);
	}
};

template<typename T>
class OwnerInputStream::Wrap final: public WrapperInputStream<T*, InputStream> {
public:
	template<typename Arg>
	Wrap(Arg&& in)
			: WrapperInputStream<T*, InputStream>(util::forward<Arg>(in)) {
	}
	Wrap(const Wrap<T>&) = delete;
	Wrap(Wrap<T> && o)
			: WrapperInputStream<T*, InputStream>(o.getWrappedPointer()) {
		o.getWrappedPointerReference() = nullptr;
	}
	~Wrap() {
		delete this->getWrappedPointer();
	}
};

template<typename T>
class OwnerOutputStream::Wrap final: public OwnerOutputStream {
	T* stream;
public:
	Wrap(T* out)
			: stream(out) {
	}
	Wrap(const Wrap<T>&) = delete;
	Wrap(Wrap<T> && o)
			: stream(o.stream) {
		o.stream = nullptr;
	}
	~Wrap() {
		delete stream;
	}

	virtual bool write(const void* data, unsigned int count) override {
		return stream->write(data, count);
	}
};

template<typename T>
class OwnerInputOutputStream::Wrap final: public OwnerInputOutputStream {
	T* stream;
public:
	Wrap(T* out)
			: stream(out) {
	}
	Wrap(const Wrap<T>&) = delete;
	Wrap(Wrap<T> && o)
			: stream(o.stream) {
		o.stream = nullptr;
	}
	~Wrap() {
		delete stream;
	}

	virtual int read(void* buffer, unsigned int count) override {
		return stream->read(buffer, count);
	}
	virtual bool write(const void* data, unsigned int count) override {
		return stream->write(data, count);
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_STREAM_OWNERSTREAM_H_ */
