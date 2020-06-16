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
 * LoopedInputStream.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_STREAM_LOOPEDINPUTSTREAM_H_
#define FRAMEWORK_IO_STREAM_LOOPEDINPUTSTREAM_H_

#include <framework/io/stream/InputStream.h>
#include <gen/log.h>

namespace rhfw {

class LoopedInputStream: public InputStream {
public:
	template<typename T>
	class Wrap;

	template<typename Arg, typename util::enable_if<util::is_rvalue_reference<Arg&&>::value>::type* = nullptr>
	static Wrap<Arg> wrap(Arg&& arg) {
		return Wrap<Arg>(util::forward<Arg>(arg));
	}
	template<typename Arg, typename util::enable_if<util::is_lvalue_reference<Arg&&>::value>::type* = nullptr>
	static Wrap<Arg&> wrap(Arg&& arg) {
		return Wrap<Arg&>(util::forward<Arg>(arg));
	}
};

template<typename T>
class LoopedInputStream::Wrap final: public WrapperInputStream<T, LoopedInputStream> {
public:
	template<typename Arg>
	Wrap(Arg&& in)
			: WrapperInputStream<T, LoopedInputStream>(util::forward<Arg>(in)) {
	}
	virtual int read(void* buffer, unsigned int count) override {
		int result = 0;
		while (result < count) {
			int res = WrapperInputStream<T, LoopedInputStream>::read(reinterpret_cast<char*>(buffer) + result, count - result);
			if (res <= 0) {
				//error or eof
				WARN(res == 0) << "LoopedInputStream end: " << res;
				return result > 0 ? result : res;
			}
			//read count
			result += res;
		}
		return result;
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_STREAM_LOOPEDINPUTSTREAM_H_ */
