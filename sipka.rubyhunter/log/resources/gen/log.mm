#include <gen/log.h>
#if LOGGING_ENABLED

#include <gen/platform.h>

#if defined(RHFW_PLATFORM_IOS) || defined(RHFW_PLATFORM_MACOSX)
#include <gen/types.h>

#import <Foundation/Foundation.h>
namespace rhfw {
template<> void __platform_log<__LoggingLevel::LEVEL_VERBOSE>(const char* fileline, const char* str) {
	NSLog(@@"VERBOSE : %s\n\t%s\n", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_DEBUG>(const char* fileline, const char* str) {
	NSLog(@@"DEBUG   : %s\n\t%s\n", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_INFO>(const char* fileline, const char* str) {
	NSLog(@@"INFO    : %s\n\t%s\n", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_WARNING>(const char* fileline, const char* str) {
	NSLog(@@"WARNING : %s\n\t%s\n", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_ERROR>(const char* fileline, const char* str) {
	NSLog(@@"ERROR   : %s\n\t%s\n", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_FAILURE>(const char* fileline, const char* str) {
	NSLog(@@"FAILURE : %s\n\t%s\n", fileline, str);
}
}  // namespace rhfw

#endif /* defined(RHFW_PLATFORM_IOS) || defined(RHFW_PLATFORM_MACOSX) */

#endif /* LOGGING_ENABLED */