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

template<__LoggingLevel Level>
class Logger {
private:
public:
	template<typename T>
	Logger& operator <<(T&& o) {
		return *this;
	}
};

} // namespace rhfw

#define WARN(condition) LOGW() << "Conditional warning: (" STRINGIZE((condition)) "): "
#define LOGTRACE() LOGD()
#define THROW() LOGWTF()
#define THROWIF(condition) LOGWTF() << "Conditional exception (" STRINGIZE((condition)) "): "
#define ASSERT(condition) LOGWTF() << "Assertion failed (" STRINGIZE((condition)) "): "

#define LOGV() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_VERBOSE>{ }
#define LOGD() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_DEBUG>{ }
#define LOGI() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_INFO>{ }
#define LOGE() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_ERROR>{ }
#define LOGW() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_WARNING>{ }
#define LOGWTF() if(false) ::rhfw::Logger<::rhfw::__LoggingLevel::LEVEL_FAILURE>{ }

#define DBGEXP(...)

#define LOG_MEMORY_LEAKS()
#define START_TRACK_MEMORY_LEAKS()

#endif /* LOG_H_ */
