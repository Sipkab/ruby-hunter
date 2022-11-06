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
 * FixedString.h
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#ifndef FIXEDSTRING_H_
#define FIXEDSTRING_H_

#include <gen/platform.h>
#include <gen/log.h>
#include <gen/fwd/types.h>
#include <gen/serialize.h>

namespace rhfw {

class FixedString {
private:
	template<typename CharType>
	static unsigned int mstrlen(const CharType* str) {
		if (str == nullptr) {
			return 0;
		}
		const CharType* p = str;
		while (*p)
			++p;
		return p - str;
	}

	unsigned int len;
	char* str;
public:
	static FixedString make(char* str, unsigned int len) {
		FixedString res;
		res.str = str;
		res.len = len;
		//move
		return static_cast<FixedString&&>(res);
	}
	static FixedString toString( int value) {
		char* buf = new char[16];
		int wr = snprintf(buf, 16, "%d", value);
		return make(buf, wr);
	}
	static FixedString toString(unsigned int value) {
		char* buf = new char[16];
		int wr = snprintf(buf, 16, "%u", value);
		return make(buf, wr);
	}
	static FixedString toString(long long value) {
		char* buf = new char[32];
		int wr = snprintf(buf, 32, "%lld", value);
		return make(buf, wr);
	}
	/*template<typename CharType, unsigned int n>
	 FixedString(const CharType (&str)[n])
	 : FixedString { str, n - 1 } {
	 }*/
	FixedString()
			: len { 0 }, str { nullptr } {
	}
	FixedString(NULLPTR_TYPE)
	: len {0}, str {nullptr} {
	}
	template<typename CharType>
	FixedString(const CharType* str)
	: FixedString {str, mstrlen(str)} {
	}
	template<typename CharType>
	FixedString(const CharType* str, unsigned int len) {
		if(len == 0) {
			this->str = nullptr;
			this->len = 0;
		} else {
			this->str = new char[len + 1];
			this->len = 0;
			for (; this->len < len && static_cast<char>(str[this->len]) != 0; ++this->len) {
				this->str[this->len] = static_cast<char>(str[this->len]);
			}
			this->str[len] = 0;
		}
	}
	FixedString(const FixedString& o)
	: len {o.len} {
		if (o.str == nullptr) {
			str = nullptr;
		} else {
			this->str = new char[len + 1];
			for (unsigned int i = 0; i < len; ++i) {
				this->str[i] = o.str[i];
			}
			this->str[len] = 0;
		}
	}
	FixedString(FixedString&& o)
	: len {o.len}, str {o.str} {
		o.len = 0;
		o.str = nullptr;
	}
	FixedString& operator=(NULLPTR_TYPE) {
		this->len = 0;
		delete[] str;
		str = nullptr;
		return *this;
	}
	FixedString& operator=(const FixedString& o) {
		*this = FixedString {o};
		return *this;
	}
	FixedString& operator=(FixedString&& o) {
		ASSERT(this != &o) << "Self move assignment";

		char* oldstr = this->str;
		this->str = o.str;
		this->len = o.len;
		o.str = nullptr;
		o.len = 0;
		delete[] oldstr;
		return *this;
	}
	~FixedString() {
		delete[] str;
	}

	template<typename CharType>
	void append(const CharType* ostr, unsigned int olen) {
		char* nstr = new char[this->len + olen + 1];
		for (unsigned int i = 0; i < this->len; ++i) {
			nstr[i] = this->str[i];
		}
		delete[] this->str;
		for (unsigned int i = 0; i < olen; ++i) {
			nstr[this->len + i] = static_cast<char>(ostr[i]);
		}
		this->len += olen;
		nstr[this->len] = 0;

		this->str = nstr;
	}

	unsigned int length() const {
		return len;
	}
	operator const char*() const {
		return str;
	}

	bool operator==(const FixedString& o) const {
		if (len != o.len || this->str == nullptr || o.str == nullptr) {
			//length doesnt match
			//or we are nullptr and other is not (because others length must be greater than 0)
			//or other is nullptr
			return false;
		}
		for (unsigned int i = 0; i < len; ++i) {
			if (str[i] != o.str[i]) {
				return false;
			}
		}
		return true;
	}
	bool operator!=(const FixedString& o) const {
		return !(*this == o);
	}
	bool operator==(NULLPTR_TYPE) const {
		return str == nullptr;
	}
	bool operator!=(NULLPTR_TYPE) const {
		return str != nullptr;
	}

	int compare(const FixedString& o) const {
		if (len == 0) {
			return o.len == 0 ? 0 : -1;
		} else if (o.len == 0) {
			return 1;
		}

		auto* a = str;
		auto* b = o.str;

		while (*a == *b && *a != 0) {
			++a;
			++b;
		}

		return *(const unsigned char*) a - *(const unsigned char*) b;
	}

	/*template<typename CharType, unsigned int N>
	 FixedString& operator+=(const CharType (&str)[N]) {
	 append(str, N);
	 return *this;
	 }*/
	FixedString& operator+=(const FixedString& o) {
		append(o.str, o.len);
		return *this;
	}
	/*template<typename CharType, unsigned int N>
	 FixedString operator+(const CharType (&str)[N]) const {
	 FixedString result;
	 result.len = this->len + N;
	 result.str = new char[result.len + 1];

	 for (unsigned int i = 0; i < this->len; ++i) {
	 result.str[i] = this->str[i];
	 }
	 for (unsigned int i = 0; i < N; ++i) {
	 result.str[this->len + i] = static_cast<char>(str[i]);
	 }
	 result.str[result.len] = 0;
	 return util::move(result);
	 }*/
	FixedString operator+(const FixedString& o) const {
		FixedString result;
		result.len = this->len + o.len;
		result.str = new char[result.len + 1];

		for (unsigned int i = 0; i < this->len; ++i) {
			result.str[i] = this->str[i];
		}
		for (unsigned int i = 0; i < o.len; ++i) {
			result.str[this->len + i] = o.str[i];
		}
		result.str[result.len] = 0;
		return util::move(result);
	}

	bool operator<(const FixedString& o) const {
		return compare(o) < 0;
	}

	char& operator[](unsigned int index) {
		ASSERT(str != nullptr);
		ASSERT(index < len);
		return str[index];
	}
	char operator[](unsigned int index) const {
		ASSERT(str != nullptr);
		ASSERT(index < len);
		return str[index];
	}

	char* begin() {
		return str;
	}
	char* end() {
		return str + len;
	}

	const char* begin() const {
		return str;
	}
	const char* end() const {
		return str + len;
	}
};

#if LOGGING_ENABLED
template<>
class __internal_tostring_t<FixedString> {
public:
	static _tostring_type tostring(const FixedString& value) {
		return (const char*) value;
	}
};
#endif /* RHFW_DEBUG */

template<unsigned int MaxSize>
class SafeFixedString {
public:
	FixedString& string;
	SafeFixedString(FixedString& string)
			: string(string) {
	}
	operator FixedString&() {
		return string;
	}
};
class IgnoreFixedString {
public:
	IgnoreFixedString(NULLPTR_TYPE) {
	}
	IgnoreFixedString() {
	}
};
class LengthOnlyFixedString {
public:
	uint32& length;
	LengthOnlyFixedString(uint32& length)
			: length(length) {
	}
	LengthOnlyFixedString(uint32&& length)
			: length(length) {
	}
};

template<unsigned int MaxSize, Endianness ENDIAN>
class SerializeExecutor<SafeFixedString<MaxSize>, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, FixedString& outdata) {
		uint32 len;
		bool success = SerializeHandler<uint32>::deserialize<ENDIAN>(is, len);
		if (!success) {
			LOGW()<< "Deserialize error";
			return false;
		}
		if (len > MaxSize) {
			LOGW()<< "String exceeded max size: " << MaxSize << " with: " << len;
			return false;
		}
		char* array = new char[len + 1];
		int read = is.read(array, len);
		WARN(read < len) << "Failed to read full string: length: " << len << " read: " << read;
		if (read >= 0) {
			array[read] = 0;
			outdata = FixedString::make(array, read);
			return true;
		}
		return false;
	}
	template<typename InStream>
	static bool deserialize(InStream& is, SafeFixedString<MaxSize>& outdata) {
		return deserialize(is, outdata.string);
	}
	template<typename InStream>
	static bool deserialize(InStream& is, SafeFixedString<MaxSize> && outdata) {
		return deserialize(is, outdata);
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SafeFixedString<MaxSize>& data) {
		return os.template serialize<FixedString, ENDIAN>(data.string);
	}
};

template<Endianness ENDIAN>
class SerializeExecutor<IgnoreFixedString, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, const IgnoreFixedString& outdata) {
		uint32 len;
		bool success = SerializeHandler<uint32>::deserialize<ENDIAN>(is, len);
		if (!success) {
			LOGW()<< "Deserialize error";
			return false;
		}
		while (len > 0) {
			char buffer[4096];
			uint32 toskip = len > sizeof(buffer) ? sizeof(buffer) : len;
			is.read(buffer, toskip);
			len -= toskip;
		}
		return true;
	}
};
template<Endianness ENDIAN>
class SerializeExecutor<LengthOnlyFixedString, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, LengthOnlyFixedString& outdata) {
		bool success = SerializeHandler<uint32>::deserialize<ENDIAN>(is, outdata.length);
		if (!success) {
			LOGW()<<"Deserialize error";
			return false;
		}
		uint32 len = outdata.length;
		while (len > 0) {
			char buffer[4096];
			uint32 toskip = len > sizeof(buffer) ? sizeof(buffer) : len;
			is.read(buffer, toskip);
			len -= toskip;
		}
		return true;
	}
	template<typename InStream>
	static bool deserialize(InStream& is, LengthOnlyFixedString&& outdata) {
		return deserialize(is, outdata);
	}
};

}
// namespace rhfw

#endif /* FIXEDSTRING_H_ */
