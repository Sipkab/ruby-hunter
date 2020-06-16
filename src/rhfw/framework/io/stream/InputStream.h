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
 * InputStream.h
 *
 *  Created on: 2016. aug. 23.
 *      Author: sipka
 */

#ifndef FRAMEWORK_IO_INPUTSTREAM_H_
#define FRAMEWORK_IO_INPUTSTREAM_H_

#include <framework/utils/utility.h>

#include <gen/fwd/types.h>
#include <gen/serialize.h>

namespace rhfw {

class InputStream {
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

	InputStream() = default;
	InputStream(InputStream&&) = default;
	InputStream(const InputStream&) = default;
	InputStream& operator=(InputStream&&) = default;
	InputStream& operator=(const InputStream&) = default;
	virtual ~InputStream() = default;

	virtual int read(void* buffer, unsigned int count) = 0;

	template<Endianness ENDIAN, typename DataType, typename ForwardType>
	bool deserialize(ForwardType&& out) {
		return SerializeHandler<DataType>::template deserialize<ENDIAN>(*this, util::forward<ForwardType>(out));
	}

	template<typename DataType, typename ForwardType, typename util::enable_if<SerializeIsEndianIndependent<DataType>::value>::type* =
			nullptr>
	bool deserialize(ForwardType&& out) {
		return SerializeHandler<DataType>::template deserialize(*this, util::forward<ForwardType>(out));
	}
};

#define WIS_PARAM(type) (*static_cast<const type*>(nullptr))
#define WIS_NCPARAM(type) (*static_cast<type*>(nullptr))

template<typename T, typename ParentType>
class WrapperInputStream: public ParentType {
protected:
	typedef typename util::remove_reference<T>::type R;
	T in;

	T& getWrapped() {
		return in;
	}
public:
	template<typename Arg, typename util::enable_if<util::is_rvalue_reference<Arg&&>::value>::type* = nullptr>
	static WrapperInputStream<Arg, InputStream> wrap(Arg&& arg) {
		return WrapperInputStream<Arg, InputStream>(util::forward<Arg>(arg));
	}
	template<typename Arg, typename util::enable_if<util::is_lvalue_reference<Arg&&>::value>::type* = nullptr>
	static WrapperInputStream<Arg&, InputStream> wrap(Arg&& arg) {
		return WrapperInputStream<Arg&, InputStream>(util::forward<Arg>(arg));
	}

	template<typename Arg>
	WrapperInputStream(Arg&& in)
			: ParentType(), in(util::forward<Arg>(in)) {
	}

	virtual int read(void* buffer, unsigned int count) override {
		return in.read(buffer, count);
	}

	template<typename U = R, typename RetType = decltype(WIS_NCPARAM(U).seek(WIS_PARAM(long long), WIS_PARAM(SeekMethod)))>
	RetType seek(long long offset, SeekMethod method) {
		return util::forward<RetType>(in.seek(offset, method));
	}
	template<typename U = R, typename RetType = decltype(WIS_NCPARAM(U).skip(WIS_PARAM(unsigned long long)))>
	RetType skip(unsigned long long count) {
		return util::forward<RetType>(in.skip(count));
	}
	template<typename U = R, typename RetType = decltype(WIS_NCPARAM(U).getPosition())>
	RetType getPosition() {
		return util::forward<RetType>(in.getPosition());
	}
};

template<typename T, typename ParentType>
class WrapperInputStream<T*, ParentType> : public ParentType {
protected:
	T* in;
	T& getWrapped() {
		return *in;
	}
	T* getWrappedPointer() {
		return in;
	}
	T*& getWrappedPointerReference() {
		return in;
	}
public:
	template<typename Arg>
	WrapperInputStream(Arg&& in)
			: ParentType(), in(util::forward<Arg>(in)) {
	}

	virtual int read(void* buffer, unsigned int count) override {
		return in->read(buffer, count);
	}

	template<typename U = T, typename RetType = decltype(WIS_NCPARAM(U).seek(WIS_PARAM(long long), WIS_PARAM(SeekMethod)))>
	RetType seek(long long offset, SeekMethod method) {
		return util::forward<RetType>(in->seek(offset, method));
	}
	template<typename U = T, typename RetType = decltype(WIS_NCPARAM(U).skip(WIS_PARAM(unsigned long long)))>
	RetType skip(unsigned long long count) {
		return util::forward<RetType>(in->skip(count));
	}
	template<typename U = T, typename RetType = decltype(WIS_NCPARAM(U).getPosition())>
	RetType getPosition() {
		return util::forward<RetType>(in->getPosition());
	}
};

#undef WIS_PARAM
#undef WIS_NCPARAM

template<Endianness ENDIAN>
class EndianInputStream: public InputStream {
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

	template<typename DataType, typename ForwardType>
	bool deserialize(ForwardType&& out) {
		return SerializeHandler<DataType>::template deserialize<ENDIAN>(*this, util::forward<ForwardType>(out));
	}
};

template<typename T>
class InputStream::Wrap final: public InputStream {
	T in;
public:
	template<typename Arg>
	Wrap(Arg&& in)
			: in(util::forward<Arg>(in)) {
	}
	virtual int read(void* buffer, unsigned int count) override {
		return in.read(buffer, count);
	}
};

template<Endianness ENDIAN>
template<typename T>
class EndianInputStream<ENDIAN>::Wrap final: public WrapperInputStream<T, EndianInputStream<ENDIAN>> {
public:
	template<typename Arg>
	Wrap(Arg&& in)
			: WrapperInputStream<T, EndianInputStream<ENDIAN>>(util::forward<Arg>(in)) {
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_IO_INPUTSTREAM_H_ */
