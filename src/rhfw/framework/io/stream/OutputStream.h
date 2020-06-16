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
 * OutputStream.h
 *
 *  Created on: 2016. aug. 23.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_OUTPUTSTREAM_H_
#define FRAMEWORK_IO_OUTPUTSTREAM_H_

#include <gen/fwd/types.h>
#include <gen/serialize.h>

namespace rhfw {

class OutputStream {
private:
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

	OutputStream() = default;
	OutputStream(OutputStream&&) = default;
	OutputStream(const OutputStream&) = default;
	OutputStream& operator=(OutputStream&&) = default;
	OutputStream& operator=(const OutputStream&) = default;
	virtual ~OutputStream() = default;

	virtual bool write(const void* data, unsigned int count) = 0;

	template<typename T>
	bool write(const T& data) {
		return write(&data, sizeof(T));
	}

	template<typename DataType, Endianness Endian>
	bool serialize(const DataType& data) {
		return SerializeHandler<DataType>::template serialize<Endian>(*this, data);
	}

	template<typename DataType, typename util::enable_if<SerializeIsEndianIndependent<DataType>::value>::type* = nullptr>
	bool serialize(const DataType& data) {
		return SerializeHandler<DataType>::template serialize(*this, data);
	}

};

template<typename T, typename ParentType>
class WrapperOutputStream: public ParentType {
protected:
	T out;
public:
	template<typename Arg, typename util::enable_if<util::is_rvalue_reference<Arg&&>::value>::type* = nullptr>
	static WrapperOutputStream<Arg, OutputStream> wrap(Arg&& arg) {
		return WrapperOutputStream<Arg, OutputStream>(util::forward<Arg>(arg));
	}
	template<typename Arg, typename util::enable_if<util::is_lvalue_reference<Arg&&>::value>::type* = nullptr>
	static WrapperOutputStream<Arg&, OutputStream> wrap(Arg&& arg) {
		return WrapperOutputStream<Arg&, OutputStream>(util::forward<Arg>(arg));
	}

	template<typename Arg>
	WrapperOutputStream(Arg&& in)
			: ParentType(), out(util::forward<Arg>(in)) {
	}
};

template<Endianness ENDIAN>
class EndianOutputStream: public OutputStream {
public:
	static const Endianness ENDIANNESS = ENDIAN;

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

	template<typename DataType, Endianness Endian = ENDIAN>
	bool serialize(const DataType& data) {
		return SerializeHandler<DataType>::template serialize<Endian>(*this, data);
	}
};

template<typename T>
class OutputStream::Wrap: public OutputStream {
	T out;
public:
	template<typename Arg>
	Wrap(Arg&& out)
			: out(util::forward<Arg>(out)) {
	}
	virtual bool write(const void* data, unsigned int count) override {
		return out.write(data, count);
	}
};

template<Endianness ENDIAN>
template<typename T>
class EndianOutputStream<ENDIAN>::Wrap: public WrapperOutputStream<T, EndianOutputStream<ENDIAN>> {
public:
	template<typename Arg>
	Wrap(Arg&& out)
			: WrapperOutputStream<T, EndianOutputStream<ENDIAN>>(util::forward<Arg>(out)) {
	}
	virtual bool write(const void* data, unsigned int count) override {
		return WrapperOutputStream<T, EndianOutputStream<ENDIAN>>::out.write(data, count);
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_OUTPUTSTREAM_H_ */
