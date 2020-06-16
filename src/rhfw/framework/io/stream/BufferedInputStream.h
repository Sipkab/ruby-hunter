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
 * BufferedInputStream.h
 *
 *  Created on: 2016. aug. 26.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_STREAM_BUFFEREDINPUTSTREAM_H_
#define FRAMEWORK_IO_STREAM_BUFFEREDINPUTSTREAM_H_

#include <framework/io/stream/InputStream.h>
#include <gen/types.h>
#include <gen/log.h>

#include <string.h>

namespace rhfw {

class BufferedInputStream: public InputStream {
private:
public:
	template<typename T>
	class Wrap;

	template<typename Arg, typename ... ISArgs, typename util::enable_if<util::is_rvalue_reference<Arg&&>::value>::type* = nullptr>
	static Wrap<Arg> wrap(Arg&& arg, ISArgs&&... isargs) {
		return Wrap<Arg>(util::forward<Arg>(arg), util::forward<ISArgs>(isargs)...);
	}
	template<typename Arg, typename ... ISArgs, typename util::enable_if<util::is_lvalue_reference<Arg&&>::value>::type* = nullptr>
	static Wrap<Arg&> wrap(Arg&& arg, ISArgs&&... isargs) {
		return Wrap<Arg&>(util::forward<Arg>(arg), util::forward<ISArgs>(isargs)...);
	}
};

template<typename T>
class BufferedInputStream::Wrap final: public WrapperInputStream<T, BufferedInputStream> {
	//the allocated size of the buffer
	unsigned int bufferSize;
	//the count of the data bytes, since the start
	unsigned int count = 0;
	//the position of the buffer, where the data starts
	unsigned int offset = 0;
	//the buffer
	char* buffer = nullptr;
public:
	template<typename Arg>
	Wrap(Arg&& in, unsigned int buffersize = 1024 * 8)
			: WrapperInputStream<T, BufferedInputStream>(util::forward<Arg>(in)), bufferSize(buffersize) {
		buffer = new char[bufferSize];
	}
	Wrap(Wrap<T> && o)
			: WrapperInputStream<T, BufferedInputStream>(util::forward<T&&>(o.in)), bufferSize(o.bufferSize), count(o.count), offset(
					o.offset), buffer(o.buffer) {
		o.buffer = nullptr;
	}
	~Wrap() {
		delete[] buffer;
	}

	bool peekEquals(const void* peekdata, unsigned int peeklen) {
		unsigned int c = count - offset;
		if (c < peeklen) {
			if (bufferSize < peeklen) {
				unsigned int nsize = bufferSize * 2;
				while (nsize < peeklen) {
					nsize *= 2;
				}
				char* nbuf = new char[nsize];
				memcpy(nbuf, this->buffer + offset, count - offset);
				offset = 0;
				count -= offset;

				delete[] buffer;
				buffer = nbuf;
				bufferSize = nsize;
			}
			//read more
			unsigned int toread = peeklen - c;
			if (bufferSize - count < toread) {
				//move to front
				memmove(buffer, buffer + offset, c);
				offset = 0;
				count -= offset;
			}
			int res = WrapperInputStream<T, BufferedInputStream>::in.read(reinterpret_cast<char*>(buffer) + count, bufferSize - count);
			if (res < 0) {
				return false;
			}
			count += res;
			if (count - offset < peeklen) {
				return false;
			}
		}
		ASSERT(count - offset >= peeklen);
		return memcmp(buffer + offset, peekdata, peeklen) == 0;
	}

	virtual int read(void* buffer, unsigned int count) override {
		int result = this->count - offset;
		if (result >= count) {
			//enough data in buffer
			memcpy(buffer, reinterpret_cast<char*>(this->buffer) + offset, count);
			this->offset += count;
			return count;
		}
		//not enough data in buffer, copy remaining
		memcpy(buffer, reinterpret_cast<char*>(this->buffer) + offset, result);
		this->count = 0;
		this->offset = 0;

		int res;
		if (count - result >= bufferSize) {
			//we would copy the whole buffer to target, read into target instead
			res = WrapperInputStream<T, BufferedInputStream>::read(reinterpret_cast<char*>(buffer) + result, count - result);
			if (res <= 0) {
				//error, or eof
				return result > 0 ? result : res;
			}
			result += res;
		} else {
			res = WrapperInputStream<T, BufferedInputStream>::read(this->buffer, bufferSize);
			if (res <= 0) {
				//error, or eof
				return result > 0 ? result : res;
			}
			if (res < count - result) {
				//less read than required
				memcpy(reinterpret_cast<char*>(buffer) + result, this->buffer, res);
				result += res;
			} else {
				//read all we needed
				memcpy(reinterpret_cast<char*>(buffer) + result, this->buffer, count - result);
				this->offset += count - result;
				this->count += res;
				result += count - result;
			}
		}

		return result;
	}

	long long seek(long long offset, SeekMethod method) {
		switch (method) {
			case SeekMethod::BEGIN: {
				const long long position = getPosition();
				const long long bufc = this->count - this->offset;
				if (position >= offset) {
					long long diff = position - offset;
					//seeking backward
					if (diff <= this->offset) {
						//reuse our buffer
						this->offset -= (int) diff;
						return position - diff;
					} else {
						//we want to seek more back than the buffer available
						this->count = this->offset = 0;
						return WrapperInputStream<T, BufferedInputStream>::seek(offset, SeekMethod::BEGIN);
					}
				} else {
					//seeking forward
					long long diff = offset - position;
					if (bufc >= diff) {
						this->offset += (int) diff;
						return position + diff;
					} else {
						this->count = this->offset = 0;
						return WrapperInputStream<T, BufferedInputStream>::seek(offset, SeekMethod::BEGIN);
					}
				}
				break;
			}
			case SeekMethod::CURRENT: {
				const long long position = getPosition();
				const long long bufc = this->count - this->offset;
				if ((offset > bufc) || ((long long) this->offset + offset < 0)) {
					//seeking forward from position, and more than we have in buffer
					//or we are seeking backward, more than we have in buffer
					this->count = this->offset = 0;
					return WrapperInputStream<T, BufferedInputStream>::seek(offset - bufc, SeekMethod::CURRENT);
				} else {
					//seeking backward, while we can reuse buffer
					//offset here is negative, or positive and we decrement count
					this->offset += (int) offset;
					return position + offset;
				}
				break;
			}
			case SeekMethod::END: {
				//we dont know where the end of the file is
				//so just seek, and clear the buffer
				WARN(offset > 0) << "Seeking past end";
				this->count = this->offset = 0;
				return WrapperInputStream<T, BufferedInputStream>::seek(offset, SeekMethod::END);
			}
			default: {
				THROW()<<"Unknown seek method: " << method;
				return -1;
			}
		}
	}
	long long skip(unsigned long long count) {
		unsigned int bufc = this->count - offset;
		if (bufc > count) {
			offset += count;
			return getPosition();
		}
		this->count = this->offset = 0;
		if (bufc == count) {
			return getPosition();
		}
		count -= bufc;
		return WrapperInputStream<T, BufferedInputStream>::skip(count);
	}
	long long getPosition() {
		return WrapperInputStream<T, BufferedInputStream>::getPosition() - (this->count - offset);
	}
};

}
// namespace rhfw

#endif /* FRAMEWORK_IO_STREAM_BUFFEREDINPUTSTREAM_H_ */
