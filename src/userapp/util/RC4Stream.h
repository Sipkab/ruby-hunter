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
 * RC4Stream.h
 *
 *  Created on: 2016. okt. 16.
 *      Author: sipka
 */

#ifndef TEST_UTIL_RC4STREAM_H_
#define TEST_UTIL_RC4STREAM_H_

#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

#include <util/RC4Cipher.h>

namespace userapp {
using namespace rhfw;

class RC4InputStream: public InputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Stream, typename ... Arg, typename util::enable_if<util::is_rvalue_reference<Stream&&>::value>::type* = nullptr>
	static Wrap<Stream> wrap(Stream&& stream, Arg&&... args) {
		return Wrap<Stream>(util::forward<Stream>(stream), util::forward<Arg>(args)...);
	}
	template<typename Stream, typename ... Arg, typename util::enable_if<util::is_lvalue_reference<Stream&&>::value>::type* = nullptr>
	static Wrap<Stream&> wrap(Stream&& stream, Arg&&... args) {
		return Wrap<Stream&>(util::forward<Stream>(stream), util::forward<Arg>(args)...);
	}

};
class RC4OutputStream: public OutputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Stream, typename ... Arg, typename util::enable_if<util::is_rvalue_reference<Stream&&>::value>::type* = nullptr>
	static Wrap<Stream> wrap(Stream&& stream, Arg&&... args) {
		return Wrap<Stream>(util::forward<Stream>(stream), util::forward<Arg>(args)...);
	}
	template<typename Stream, typename ... Arg, typename util::enable_if<util::is_lvalue_reference<Stream&&>::value>::type* = nullptr>
	static Wrap<Stream&> wrap(Stream&& stream, Arg&&... args) {
		return Wrap<Stream&>(util::forward<Stream>(stream), util::forward<Arg>(args)...);
	}
};

template<typename T>
class RC4InputStream::Wrap final: public WrapperInputStream<T, RC4InputStream> {
	RC4Cipher& cipher;
public:
	template<typename Arg>
	Wrap(Arg&& in, RC4Cipher& cipher)
			: WrapperInputStream<T, RC4InputStream>(util::forward<Arg>(in)), cipher(cipher) {
	}
	virtual int read(void* buffer, unsigned int count) override {
		int res = WrapperInputStream<T, RC4InputStream>::read(buffer, count);
		if (res > 0) {
			cipher.decrypt(reinterpret_cast<uint8*>(buffer), res);
		}
		return res;
	}
};

template<typename T>
class RC4OutputStream::Wrap final: public WrapperOutputStream<T, RC4OutputStream> {
	static const unsigned int BUFFER_SIZE = 4096;

	RC4Cipher& cipher;
	uint8 buffer[BUFFER_SIZE];
public:
	template<typename Arg>
	Wrap(Arg&& in, RC4Cipher& cipher)
			: WrapperOutputStream<T, RC4OutputStream>(util::forward<Arg>(in)), cipher(cipher) {
	}
	virtual bool write(const void* data, unsigned int count) override {
		while (count > 0) {
			unsigned int c = count > BUFFER_SIZE ? BUFFER_SIZE : count;
			cipher.encrypt(reinterpret_cast<const uint8*>(data), buffer, c);
			bool res = WrapperOutputStream<T, RC4OutputStream>::out.write(buffer, c);
			if (!res) {
				return false;
			}
			count -= c;
			reinterpret_cast<const uint8*&>(data) += c;
		}
		return true;
	}
};

}  // namespace userapp

#endif /* TEST_UTIL_RC4STREAM_H_ */
