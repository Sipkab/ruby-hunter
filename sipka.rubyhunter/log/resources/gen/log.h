#ifndef LOG_H_
#define LOG_H_

#include <framework/utils/utility.h>
#include <gen/configuration.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define FILE_LINE __FILE__ ":" LINE_STRING
#define DEBUG_FILE_LINE ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_VERBOSE>::shortenFileLine(FILE_LINE)

#define LOGGING_ENABLED 1

namespace rhfw {
enum class __LoggingLevel
	: unsigned int {
	LEVEL_VERBOSE = 0x0, /* 0 */
	LEVEL_DEBUG = 0x1, /* 1 */
	LEVEL_INFO = 0x2, /* 2 */
	LEVEL_WARNING = 0x3, /* 3 */
	LEVEL_ERROR = 0x4, /* 4 */
	LEVEL_FAILURE = 0x5, /* 5 */
	_count_of_entries = 6
};
class _tostring_type {
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
private:
	char* str;
public:
	_tostring_type(const char* str, size_t len) {
		if(str == nullptr){
			str = "(nullptr)";
			len = 9;
		}
		this->str = (char*) malloc((len + 1) * sizeof(char));
		for (size_t i = 0; i < len; ++i) {
			this->str[i] = str[i];
		}
		this->str[len] = 0;
	}
	_tostring_type(const wchar_t* str, size_t len) {
		if (str == nullptr) {
			str = L"(nullptr)";
			len = 9;
		}
		this->str = (char*) malloc((len + 1) * sizeof(char));
		for (size_t i = 0; i < len; ++i) {
			this->str[i] = static_cast<char>(str[i]);
		}
		this->str[len] = 0;
	}
	_tostring_type(const char* str)
			: _tostring_type { str, mstrlen(str) } {
	}
	_tostring_type(const wchar_t* str)
			: _tostring_type { str, mstrlen(str) } {
	}
	template<unsigned int N>
	_tostring_type(const char (&a)[N])
			: _tostring_type { a, N - 1 } {
	}
	template<unsigned int N>
	_tostring_type(char (&a)[N])
			: _tostring_type { a, N - 1 } {
	}
	_tostring_type(_tostring_type&& o)
			: str { o.str } {
		o.str = nullptr;
	}
	_tostring_type(const _tostring_type&) = delete;
	_tostring_type& operator=(const _tostring_type&) = delete;
	_tostring_type& operator=(_tostring_type&& o) {
		free(str);
		this->str = o.str;
		o.str = nullptr;
		return *this;
	}
	~_tostring_type() {
		free(str);
	}

	_tostring_type operator+(const _tostring_type& o) const {
		size_t alen = mstrlen((const char*)*this);
		size_t blen = mstrlen((const char*)o);
		char* res = (char*) malloc((alen + blen + 1) * sizeof(char));
		for (size_t i = 0; i < alen; ++i) {
			res[i] = (*this)[i];
		}
		for (size_t i = 0; i < blen; ++i) {
			res[alen + i] = o[i];
		}
		res[alen + blen] = 0;
		return _tostring_type { res };
	}

	operator const char*() const {
		return str;
	}
};

template<typename T>
_tostring_type __internal_tostring(const T& val);

template<typename T> class __internal_tostring_t {
public:
	static _tostring_type tostring(const T& value) {
		return __internal_tostring<T>(value);
	}
};
template<typename T>
class __internal_tostring_t<const T> : public __internal_tostring_t<T> {
};

template<typename T> class __internal_tostring_t<const T*> {
public:
	static _tostring_type tostring(const T* value) {
		char buf[64];
		sprintf(buf, "%p", value);
		return buf;
	}
};
template<typename T> class __internal_tostring_t<T*> {
public:
	static _tostring_type tostring(const T* value) {
		char buf[64];
		sprintf(buf, "%p", value);
		return buf;
	}
};
template<typename T, unsigned int N>
class __internal_tostring_t<T[N]> : public __internal_tostring_t<const T*> {
};
template<typename T, unsigned int N>
class __internal_tostring_t<const T[N]> : public __internal_tostring_t<const T*> {
};

template<unsigned int N>
class __internal_tostring_t<const char[N]> {
public:
	static _tostring_type tostring(const char (&s)[N]) {
		return s;
	}
};
template<>
class __internal_tostring_t<const char*> {
public:
	static _tostring_type tostring(const char * s) {
		return _tostring_type { s };
	}
};
template<>
class __internal_tostring_t<char*> {
public:
	static _tostring_type tostring(char* s) {
		return _tostring_type { s };
	}
};

template<>
class __internal_tostring_t<const unsigned char*> {
public:
	static _tostring_type tostring(const unsigned char * s) {
		return _tostring_type { reinterpret_cast<const char*>(s) };
	}
};
template<>
class __internal_tostring_t<unsigned char*> {
public:
	static _tostring_type tostring(unsigned char* s) {
		return _tostring_type { reinterpret_cast<const char*>(s) };
	}
};
template<>
class __internal_tostring_t<const wchar_t*> {
public:
	static _tostring_type tostring(const wchar_t * s) {
		return _tostring_type { s };
	}
};
template<>
class __internal_tostring_t<wchar_t*> {
public:
	static _tostring_type tostring(wchar_t* s) {
		return _tostring_type { s };
	}
};

template<> class __internal_tostring_t<unsigned char> {
public:
	static _tostring_type tostring(unsigned char value) {
		char buf[4];
		sprintf(buf, "%c", value);
		return buf;
	}
};
template<> class __internal_tostring_t<char> {
public:
	static _tostring_type tostring(char value) {
		char buf[4];
		sprintf(buf, "%c", value);
		return buf;
	}
};
template<> class __internal_tostring_t<signed char> {
public:
	static _tostring_type tostring(signed char value) {
		char buf[4];
		sprintf(buf, "%c", value);
		return buf;
	}
};
template<> class __internal_tostring_t<unsigned short> {
public:
	static _tostring_type tostring(unsigned short value) {
		char buf[16];
		sprintf(buf, "%hu", value);
		return buf;
	}
};
template<> class __internal_tostring_t<short> {
public:
	static _tostring_type tostring(short value) {
		char buf[16];
		sprintf(buf, "%hd", value);
		return buf;
	}
};
template<> class __internal_tostring_t<unsigned int> {
public:
	static _tostring_type tostring(unsigned int value) {
		char buf[32];
		sprintf(buf, "%u", value);
		return buf;
	}
};
template<> class __internal_tostring_t<int> {
public:
	static _tostring_type tostring(int value) {
		char buf[32];
		sprintf(buf, "%d", value);
		return buf;
	}
};
template<> class __internal_tostring_t<unsigned long> {
public:
	static _tostring_type tostring(unsigned long value) {
		char buf[64];
		sprintf(buf, "%lu", value);
		return buf;
	}
};
template<> class __internal_tostring_t<long> {
public:
	static _tostring_type tostring(long value) {
		char buf[64];
		sprintf(buf, "%ld", value);
		return buf;
	}
};
template<> class __internal_tostring_t<unsigned long long> {
public:
	static _tostring_type tostring(unsigned long long value) {
		char buf[64];
		sprintf(buf, "%llu", value);
		return buf;
	}
};
template<> class __internal_tostring_t<long long> {
public:
	static _tostring_type tostring(long long value) {
		char buf[64];
		sprintf(buf, "%lld", value);
		return buf;
	}
};

template<> class __internal_tostring_t<float> {
public:
	static _tostring_type tostring(float value) {
		char buf[64];
		sprintf(buf, "%f", value);
		return buf;
	}
};
template<> class __internal_tostring_t<double> {
public:
	static _tostring_type tostring(double value) {
		char buf[64];
		sprintf(buf, "%f", value);
		return buf;
	}
};
template<> class __internal_tostring_t<bool> {
public:
	static _tostring_type tostring(bool value) {
		if (value)
			return "true";
		return "false";
	}
};

template<__LoggingLevel Level> void __platform_log(const char* fileline, const char* str);
class __logger_action_void {
private:
public:
	static void finish() {
	}
};
class __logger_action_aborter {
private:
public:
	static void finish() {
		abort();
	}
};
template<typename T>
_tostring_type TOSTRING(T&& o) {
	return __internal_tostring_t<typename util::remove_reference<T>::type>::tostring(util::forward < T > (o));
}

template<__LoggingLevel Level, typename FinishAction = __logger_action_void> class Logger {
private:
	const char* fline;
	unsigned int allocated;
	char* str;
	unsigned int count;

	void ensure(unsigned int remain) {
		if (allocated - count - 1 < remain) {
			while (allocated - count - 1 < remain) {
				allocated *= 2;
			}
			char* nstr = (char*) malloc(allocated * sizeof(char));
			for (unsigned int i = 0; i < count; ++i) {
				nstr[i] = str[i];
			}
			free(str);
			str = nstr;
		}
	}
public:
	Logger(const char* fline)
	: fline {fline}, allocated {512}, str {(char*)malloc(allocated * sizeof(char))}, count {0} {
		str[0] = 0;
	}
	Logger(Logger&& o)
	: fline {o.fline}, allocated {o.allocated}, str {o.str}, count {o.count} {
		o.str = nullptr;
		o.allocated = 0;
		o.count = 0;
	}
	Logger& operator=(Logger&&) = delete;
	Logger(const Logger*) = delete;
	Logger& operator=(const Logger*) = delete;
	~Logger() {
		if (str != nullptr) {
			__platform_log<Level>(fline, str);
			free(str);
			FinishAction::finish();
		}
	}

	template<typename T>
	Logger operator <<(T&& o) && {
		auto s = TOSTRING(util::forward < T > (o));
		size_t len = strlen(s);
		ensure(len);
		for (size_t i = 0; i < len; ++i) {
			str[count++] = s[i];
		}
		str[count] = 0;
		return util::move(*this);
	}

	template<unsigned int N>
	static const char* shortenFileLine(const char (&str)[N]){
		unsigned int i = N - 1;
		while(i != 0){
			if(str[i] == '\\' || str[i] == '/'){
				return str + i + 1;
			}
			--i;
		}
		return str;
	}
};
} // namespace rhfw

#define WARN(condition) if(condition) LOGW() << "Conditional warning: (" STRINGIZE((condition)) "): "
#define LOGTRACE() LOGD() << __func__ << " "
#define THROW() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_FAILURE, ::rhfw::__logger_action_aborter>{ DEBUG_FILE_LINE } << "Exception in function: " << __func__ << ": "
#define THROWIF(condition) if (condition)  THROW() << "Conditional exception (" STRINGIZE((condition)) "): "
#define ASSERT(condition) if (!(condition)) THROW() << "Assertion failed (" STRINGIZE((condition)) "): "

#define LOGV() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_VERBOSE>{ DEBUG_FILE_LINE }
#define LOGD() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_DEBUG>{ DEBUG_FILE_LINE }
#define LOGI() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_INFO>{ DEBUG_FILE_LINE }
#define LOGE() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_ERROR>{ DEBUG_FILE_LINE }
#define LOGW() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_WARNING>{ DEBUG_FILE_LINE }
#define LOGWTF() ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_FAILURE>{ DEBUG_FILE_LINE }

#define DBGEXP(...) __VA_ARGS__

@if this.Debug && this.debugNew && this.supportDebugNew:
void* operator new(size_t count, const char* filename, int line) ;
void* operator new[](size_t count, const char* filename, int line) ;
void operator delete(void* ptr, const char* file, int line)  ;
void operator delete[](void* ptr, const char* file, int line)  ;

namespace rhfw {
void do_memory_leak_logging();
void start_memory_leak_logging();
} // namespace rhfw

#define new new(__FILE__, __LINE__)
#define LOG_MEMORY_LEAKS() rhfw::do_memory_leak_logging()
#define START_TRACK_MEMORY_LEAKS() rhfw::start_memory_leak_logging()
@else:
#define LOG_MEMORY_LEAKS()
#define START_TRACK_MEMORY_LEAKS()
@

#endif /* LOG_H_ */
