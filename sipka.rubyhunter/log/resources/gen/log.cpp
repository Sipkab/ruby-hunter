#include <gen/log.h>
#include <gen/types.h>
#include <gen/platform.h>

@if this.enabled :
	@foreach type in this.typeDeclarations.values() :
@opt type.toSourceString()@
@	@

@if this.enabled && this.debugNew && this.supportDebugNew:
	@include:debug_new.cpp@
@

@if this.enabled :
#if defined(RHFW_PLATFORM_ANDROID)
#include <android/log.h>
namespace rhfw {
template<> void __platform_log<__LoggingLevel::LEVEL_VERBOSE>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_VERBOSE, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_DEBUG>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_DEBUG, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_INFO>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_INFO, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_WARNING>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_WARN, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_ERROR>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_ERROR, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_FAILURE>(const char* fileline, const char* str) {
	__android_log_write(ANDROID_LOG_FATAL, fileline, str);
}
}  // namespace rhfw

#elif defined(RHFW_PLATFORM_IOS) || defined(RHFW_PLATFORM_MACOSX)

//see log.mm

#elif defined(RHFW_PLATFORM_LINUX)
#include <stdio.h>
#include <time.h>
namespace rhfw {
template<> void __platform_log<__LoggingLevel::LEVEL_VERBOSE>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: VERBOSE : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_DEBUG>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: DEBUG   : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_INFO>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: INFO    : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_WARNING>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: WARNING : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_ERROR>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: ERROR   : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_FAILURE>(const char* fileline, const char* str) {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	long long millis = (static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000) / 1000;
	printf("%llu.%03llu: FAILURE : %s\n\t%s\n", millis / 1000, millis % 1000, fileline, str);
}
}  // namespace rhfw

#elif defined(RHFW_PLATFORM_WINDOWSSTORE) || defined(RHFW_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace rhfw {
struct PFrequency {
	LARGE_INTEGER performancefrequency;
	PFrequency() {
		QueryPerformanceFrequency(&performancefrequency);
	}
};
static void do_debug_log(const char* level, const char* fileline, const char* str) {
	static PFrequency pf;
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	time.QuadPart *= 1000000;
	time.QuadPart /= pf.performancefrequency.QuadPart;

	printf("%llu.%03llu: %s%s\n\t%s\n", time.QuadPart / 1000000, (time.QuadPart / 1000) % 1000, level, fileline, str);
	OutputDebugStringA(level);
	OutputDebugStringA(fileline);
	OutputDebugStringA("\n\t");
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}
template<> void __platform_log<__LoggingLevel::LEVEL_VERBOSE>(const char* fileline, const char* str) {
	do_debug_log("VERBOSE: ", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_DEBUG>(const char* fileline, const char* str) {
	do_debug_log("DEBUG  : ", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_INFO>(const char* fileline, const char* str) {
	do_debug_log("INFO   : ", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_WARNING>(const char* fileline, const char* str) {
	do_debug_log("WARNING: ", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_ERROR>(const char* fileline, const char* str) {
	do_debug_log("ERROR  : ", fileline, str);
}
template<> void __platform_log<__LoggingLevel::LEVEL_FAILURE>(const char* fileline, const char* str) {
	do_debug_log("FAILURE: ", fileline, str);
}

} // namespace rhfw

#else
#if LOGGING_ENABLED
static_assert(false, "unknown platform");
#endif /* LOGGING_ENABLED */
#endif /* defined(RHFW_PLATFORM_ANDROID) */
@
