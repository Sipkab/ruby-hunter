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
 * appglue.cpp
 *
 *  Created on: 2015 aug. 11
 *      Author: sipka
 */

#include <androidplatform/AndroidPlatform.h>
#include <framework/core/AppInterface.h>
#include <framework/core/Window.h>
#include <framework/core/timing.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>

#include <android/configuration.h>
#include <android/api-level.h>
#include <android/looper.h>
#include <android/input.h>
#include <android/native_activity.h>
#include <android/log.h>
#include <androidplatform/appglue.h>

#include <gen/log.h>
//#include <gen/modules.h>
#include <gen/types.h>
//#include <gen/resconfig.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
namespace rhfw {

class platform_bridge {
public:
	static void application_main(void*);

	static bool executeDrawing();

	static int looperInputQueueFdCallback(int fd, int events, void* data);
	static int looperCommandFdCallback(int fd, int events, void* data);

	static void handleMotionInput(AInputEvent* event, core::AndroidWindow* window);
	static void handleKeyInput(AInputEvent* event, core::AndroidWindow* window, int32_t keycode);
	static void handleAppInput(AInputEvent* event, core::AndroidWindow* window);

	static void initializeFirstActivity(ANativeActivity* activity);
};

} // namespace rhfw

using namespace rhfw;
//TODO close valtozot a stackra kell rakni
class AndroidMainThreadState {
	int msgpipe[2] { 0, 0 };
	struct timespec currentTime { 0 };
	bool initialized = false;

	void refreshTime() {
		int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
		ASSERT(res == 0) << "failed to get time error: " << strerror(errno);
	}
public:
	Semaphore commandCompletedSemaphore { Semaphore::auto_init { } };
	ALooper* looper = nullptr;

	AConfiguration* configuration = nullptr;

	jclass KeyCharacterMapClass = nullptr;
	jmethodID getDeadCharMethodId = nullptr;

	AndroidMainThreadState() {
		configuration = AConfiguration_new();

		int res;

		res = pipe(msgpipe);
		ASSERT(res == 0) << "failed to create pipe, errno: " << strerror(errno);
	}
	~AndroidMainThreadState() {
		int res;
		res = close(msgpipe[0]);
		ASSERT(res == 0) << "failed to close pipe, errno: " << strerror(errno);
		res = close(msgpipe[1]);
		ASSERT(res == 0) << "failed to close pipe, errno: " << strerror(errno);

		AConfiguration_delete(configuration);
	}

	int getPipeReadFd() const {
		return msgpipe[0];
	}
	int getPipeWriteFd() const {
		return msgpipe[1];
	}

	core::time_micros getMonotonicTime() {
		refreshTime();
		return core::time_micros { static_cast<long long>(currentTime.tv_sec) * 1000000 + static_cast<long long>(currentTime.tv_nsec) / 1000 };
	}

	void writeAppData(const void* data, int count) {
		if (write(this->getPipeWriteFd(), data, count) != count) {
			LOGE()<< "Failure writing android_app cmd: " << strerror(errno);
			THROW() << "Failed to write android app data";
		}
		this->commandCompletedSemaphore.wait();
	}

	void initialize() {
		if (initialized) {
			return;
		}
		initialized = true;

		//TODO
		//::rhfw::module::initialize_modules();

		user_app_initialize(0, nullptr);

		JNIEnv& env = *Thread::getJniEnvForCurrentThread();
		jclass kcmClass = env.FindClass("android/view/KeyCharacterMap");
		getDeadCharMethodId = env.GetStaticMethodID(kcmClass, "getDeadChar", "(II)I");
		KeyCharacterMapClass = (jclass) env.NewGlobalRef(kcmClass);
		ASSERT(getDeadCharMethodId != nullptr) << "Failed to get method id";
		ASSERT(KeyCharacterMapClass != nullptr) << "Failed to create new global reference";
		env.DeleteLocalRef(kcmClass);
	}
	void destroy() {
		if (!initialized) {
			return;
		}
		initialized = false;

		if (KeyCharacterMapClass != nullptr) {
			JNIEnv& env = *Thread::getJniEnvForCurrentThread();
			env.DeleteGlobalRef(KeyCharacterMapClass);
		}
	}
};

enum {
	APP_DRAW,
	/**
	 * Command from main thread: the AInputQueue has changed.  Upon processing
	 * this command, android_app->inputQueue will be updated to the new queue
	 * (or NULL).
	 */
	APP_CMD_INPUT_CHANGED,

	/**
	 * Command from main thread: a new ANativeWindow is ready for use.  Upon
	 * receiving this command, android_app->window will contain the new window
	 * surface.
	 */
	APP_CMD_WINDOW_CREATED,

	/**
	 * Command from main thread: the existing ANativeWindow needs to be
	 * terminated.  Upon receiving this command, android_app->window still
	 * contains the existing window; after calling android_app_exec_cmd
	 * it will be set to NULL.
	 */
	APP_CMD_WINDOW_DESTROYING,

	/**
	 * Command from main thread: the current ANativeWindow has been resized.
	 * Please redraw with its new size.
	 */
	APP_CMD_WINDOW_RESIZED,

	/**
	 * Command from main thread: the system needs that the current ANativeWindow
	 * be redrawn.  You should redraw the window before handing this to
	 * android_app_exec_cmd() in order to avoid transient drawing glitches.
	 */
	APP_CMD_WINDOW_REDRAW_NEEDED,

	/**
	 * Command from main thread: the content area of the window has changed,
	 * such as from the soft input window being shown or hidden.  You can
	 * find the new content rect in android_app::contentRect.
	 */
	APP_CMD_CONTENT_RECT_CHANGED,

	/**
	 * Command from main thread: the app's activity window has gained
	 * input focus.
	 */
	APP_CMD_GAINED_FOCUS,

	/**
	 * Command from main thread: the app's activity window has lost
	 * input focus.
	 */
	APP_CMD_LOST_FOCUS,

	/**
	 * Command from main thread: the current device configuration has changed.
	 */
	APP_CMD_CONFIG_CHANGED,

	/**
	 * Command from main thread: the system is running low on memory.
	 * Try to reduce your memory use.
	 */
	APP_CMD_LOW_MEMORY,

	/**
	 * Command from main thread: the app's activity has been created.
	 */
	APP_CMD_CREATE,

	/**
	 * Command from main thread: the app's activity has been started.
	 */
	APP_CMD_START,

	/**
	 * Command from main thread: the app's activity has been resumed.
	 */
	APP_CMD_RESUME,

	/**
	 * Command from main thread: the app should generate a new saved state
	 * for itself, to restore from later if needed.  If you have saved state,
	 * allocate it with malloc and place it in android_app.savedState with
	 * the size in android_app.savedStateSize.  The will be freed for you
	 * later.
	 */
	APP_CMD_SAVE_STATE,

	/**
	 * Command from main thread: the app's activity has been paused.
	 */
	APP_CMD_PAUSE,

	/**
	 * Command from main thread: the app's activity has been stopped.
	 */
	APP_CMD_STOP,

	/**
	 * Command from main thread: the app's activity is being destroyed,
	 * and waiting for the app thread to clean up and exit before proceeding.
	 */
	APP_CMD_DESTROY,

	/**
	 * Command from main thread:
	 * there is multiple unicode character input coming
	 */
	APP_CMD_MULTIPLE_UNICODE_INPUT,

	/**
	 * Command from main thread:
	 * the device display rotation is changed
	 */
	APP_CMD_DISPLAY_ORIENTATION_CHANGED,

};

static jmethodID setNativePointerMethodId = nullptr;
static jfieldID xdpiFieldId = nullptr;
static jfieldID ydpiFieldId = nullptr;
static jmethodID getUnicodeFromKeyMethodId = nullptr;
static AndroidMainThreadState mainThreadState;

static class MainThreadCloser {
public:
	//TODO concurrency checks
	bool closed = false;

	~MainThreadCloser() {
		closed = true;
		ALooper* looper = mainThreadState.looper;
		if (looper != nullptr) {
			ALooper_wake(looper);
		}
	}
} main_thread_closer;

static void print_cur_config(AConfiguration* config) {
	char lang[2], country[2];
	AConfiguration_getLanguage(config, lang);
	AConfiguration_getCountry(config, country);
	LOGV()<< "Config changed " << "keys: " << AConfiguration_getKeyboard(config) << " keyshidde: " << AConfiguration_getKeysHidden(config);

	/*
	 LOGV(
	 "Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d modetype=%d modenight=%d",
	 AConfiguration_getMcc(config), AConfiguration_getMnc(config), lang[0], lang[1], country[0], country[1],
	 AConfiguration_getOrientation(config), AConfiguration_getTouchscreen(config), AConfiguration_getDensity(config),
	 AConfiguration_getKeyboard(config), AConfiguration_getNavigation(config), AConfiguration_getKeysHidden(config),
	 AConfiguration_getNavHidden(config), AConfiguration_getSdkVersion(config), AConfiguration_getScreenSize(config),
	 AConfiguration_getScreenLong(config), AConfiguration_getUiModeType(config), AConfiguration_getUiModeNight(config));
	 */
}

template<typename Messagetype>
class CommandMessage;

//packing can cause crashes

template<>
class /*__attribute__((__packed__))*/CommandMessage<void> {
public:
	unsigned int cmd;
	ANativeActivity* activity;
};
typedef CommandMessage<void> EmptyCommandMessage;

template<typename Messagetype>
class /*__attribute__((__packed__))*/CommandMessage {
public:
	CommandMessage<void> msg;
	Messagetype data;
};

class /*__attribute__((__packed__))*/ActivityCreateMessage {
public:
	float xdpi;
	float ydpi;
};

class /*__attribute__((__packed__))*/WindowCommandMessage {
public:
	ANativeWindow* window;
};
class /*__attribute__((__packed__))*/InputChangedCommandMessage {
public:
	AInputQueue* queue;
};
class /*__attribute__((__packed__))*/ContentRectChangedCommandMessage {
public:
	ARect rect;
};
class /*__attribute__((__packed__))*/UnicodeInputCommandMessage {
public:
	UnicodeCodePoint* codepoints;
	unsigned int count;
};
class /*__attribute__((__packed__))*/DisplayOrientationChangedMessage {
public:
	UIRotation rotation;
};
// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------
inline static void writeAppData(const void* data, unsigned int count) {
	mainThreadState.writeAppData(data, count);
}
template<typename T>
static void writeAppData(const T& data) {
	writeAppData(&data, sizeof(T));
}

namespace rhfw {
namespace androidplatform {
static AAssetManager* assetManager = nullptr;
static unsigned int sdkVersion = 0;

//XXX temporary
jclass nativeActivityClass = 0;

AAssetManager* getAssetManager() {
	return assetManager;
}

unsigned int getSdkVersion() {
	return sdkVersion;
}

UIRotation queryDeviceRotation(JNIEnv& env) {
	static jmethodID rotationmethodid = env.GetStaticMethodID(androidplatform::nativeActivityClass, "getDisplayRotationDegrees", "()I");
	jint result = env.CallStaticIntMethod(androidplatform::nativeActivityClass, rotationmethodid);
	return (UIRotation) (result / (int) UIRotation::ROTATION_DEGREE);
}

LinkedList<AndroidDeviceOrientationChangedListener, false> androidDeviceOrientationChangedEvents;
static bool deviceRotationChangedNativeRegistered = false;
static void deviceRotationChanged_native(JNIEnv* env, jclass actvityclass, jint degrees) {
	UIRotation rotation = (UIRotation) (degrees / (int) UIRotation::ROTATION_DEGREE);
	LOGI()<< "Device display rotation changed: " << rotation;

	writeAppData(CommandMessage<DisplayOrientationChangedMessage> { APP_CMD_DISPLAY_ORIENTATION_CHANGED, nullptr, rotation });
}

UIRotation subscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener) {
	ASSERT(sdkVersion >= 17);
	UIRotation result;
	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	if (androidDeviceOrientationChangedEvents.isEmpty()) {
		if (!deviceRotationChangedNativeRegistered) {
			const JNINativeMethod methods[] = { { "deviceRotationChanged", "(I)V", (void*) deviceRotationChanged_native }, };
			env.RegisterNatives(nativeActivityClass, methods, sizeof(methods) / sizeof(const JNINativeMethod));
			deviceRotationChangedNativeRegistered = true;
		}

		static jmethodID subscribeMethod = env.GetStaticMethodID(nativeActivityClass, "subscribeOrientation", "()I");
		jint rot = env.CallStaticIntMethod(nativeActivityClass, subscribeMethod);
		result = (UIRotation) (rot / (int) UIRotation::ROTATION_DEGREE);
	} else {
		result = queryDeviceRotation(env);
	}
	androidDeviceOrientationChangedEvents.addToEnd(*listener);
	return result;
}
void unsubscribeDeviceOrientationChange(AndroidDeviceOrientationChangedListener* listener) {
	ASSERT(sdkVersion >= 17);
	ASSERT(!androidDeviceOrientationChangedEvents.isEmpty());
	listener->removeLinkFromList();
	if (androidDeviceOrientationChangedEvents.isEmpty()) {
		JNIEnv& env = *Thread::getJniEnvForCurrentThread();
		static jmethodID unsubscribeMethod = env.GetStaticMethodID(nativeActivityClass, "unsubscribeOrientation", "()V");
		env.CallStaticVoidMethod(nativeActivityClass, unsubscribeMethod);
	}
}

} // namespace android
} // namespace rhfw

static void onDestroy(ANativeActivity* activity) {
	//LOGV("Destroy: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_DESTROY, activity });
	if (!core::Window::hasWindows()) {
		LOG_MEMORY_LEAKS();
	}
}

static void onStart(ANativeActivity* activity) {
	//LOGV("Start: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_START, activity });
}

static void onResume(ANativeActivity* activity) {
	//LOGV("Resume: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_RESUME, activity });
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen) {
	//LOGV("onSaveInstanceState: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_SAVE_STATE, activity });
	return nullptr;
}

static void onPause(ANativeActivity* activity) {
	//LOGV("Pause: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_PAUSE, activity });
}

static void onStop(ANativeActivity* activity) {
	//LOGV("Stop: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_STOP, activity });
}

static void onConfigurationChanged(ANativeActivity* activity) {
	//LOGV("ConfigurationChanged: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_CONFIG_CHANGED, activity });

}

static void onLowMemory(ANativeActivity* activity) {
	//LOGV("LowMemory: %p tid: %d", activity, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_LOW_MEMORY, activity });
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
	//LOGV("WindowFocusChanged: %p -- %d tid: %d", activity, focused, gettid());
	writeAppData(EmptyCommandMessage { focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, activity });
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
	//LOGV("NativeWindowCreated: %p -- %p tid: %d", activity, window, gettid());
	writeAppData(CommandMessage<WindowCommandMessage> { APP_CMD_WINDOW_CREATED, activity, window });
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
	//LOGV("NativeWindowDestroyed: %p -- %p tid: %d", activity, window, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_WINDOW_DESTROYING, activity });
}

static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
	//LOGV("onNativeWindowResized: %p -- %p tid: %d", activity, window, gettid());e
	writeAppData(EmptyCommandMessage { APP_CMD_WINDOW_RESIZED, activity });
}

static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window) {
	//LOGV("onNativeWindowRedrawNeeded: %p -- %p tid: %d", activity, window, gettid());
	writeAppData(EmptyCommandMessage { APP_CMD_WINDOW_REDRAW_NEEDED, activity });
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
	//LOGV("InputQueueCreated: %p -- %p tid: %d", activity, queue, gettid());
	writeAppData(CommandMessage<InputChangedCommandMessage> { APP_CMD_INPUT_CHANGED, activity, queue });
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
	//LOGV("InputQueueDestroyed: %p -- %p tid: %d", activity, queue, gettid());
	writeAppData(CommandMessage<InputChangedCommandMessage> { APP_CMD_INPUT_CHANGED, activity, nullptr });
}

static void onContentRectChanged(ANativeActivity* activity, const ARect* rect) {
	//LOGV("onContentRectChanged: %p -- %p tid: %d", activity, rect, gettid());

	writeAppData(CommandMessage<ContentRectChangedCommandMessage> { APP_CMD_CONTENT_RECT_CHANGED, activity, *rect });
}
/*
 static void applyPlatformConfigurations(AConfiguration* aconf, ResourceConfiguration& target, int32_t difference) {
 if (HAS_FLAG(difference, ACONFIGURATION_LOCALE)) {

 }
 if (HAS_FLAG(difference, ACONFIGURATION_ORIENTATION)) {

 }
 }

 void rhfw::applyPlatformConfigurations(ResourceConfiguration& config) {
 ASSERT(mainThreadState.configuration != nullptr) << "AConfiguration is not set";
 applyPlatformConfigurations(mainThreadState.configuration, config, 0xFFFFFFFF);
 }
 */

static void handleConfigChanges(AConfiguration* newconfig, int32_t difference) {
	print_cur_config(newconfig);
	//ResourceConfiguration updateconf;
	//applyPlatformConfigurations(newconfig, updateconf, difference);

}

static const KeyCode KEYCODE_MAP[] = {
/* AKEYCODE_UNKNOWN        	 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SOFT_LEFT       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SOFT_RIGHT      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_HOME            				*/KeyCode::KEY_UNKNOWN,/* never delivered */
/* AKEYCODE_BACK            				*/KeyCode::KEY_BACK,
/* AKEYCODE_CALL            				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ENDCALL         				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_0               				*/KeyCode::KEY_0,
/* AKEYCODE_1               				*/KeyCode::KEY_1,
/* AKEYCODE_2               				*/KeyCode::KEY_2,
/* AKEYCODE_3               				*/KeyCode::KEY_3,
/* AKEYCODE_4               				*/KeyCode::KEY_4,
/* AKEYCODE_5               				*/KeyCode::KEY_5,
/* AKEYCODE_6               				*/KeyCode::KEY_6,
/* AKEYCODE_7               				*/KeyCode::KEY_7,
/* AKEYCODE_8               				*/KeyCode::KEY_8,
/* AKEYCODE_9               				*/KeyCode::KEY_9,
/* AKEYCODE_STAR            				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_POUND           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_DPAD_UP         				*/KeyCode::KEY_DIR_UP,
/* AKEYCODE_DPAD_DOWN       				*/KeyCode::KEY_DIR_DOWN,
/* AKEYCODE_DPAD_LEFT       				*/KeyCode::KEY_DIR_LEFT,
/* AKEYCODE_DPAD_RIGHT      				*/KeyCode::KEY_DIR_RIGHT,
/* AKEYCODE_DPAD_CENTER     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_VOLUME_UP       				*/KeyCode::KEY_VOLUME_UP,
/* AKEYCODE_VOLUME_DOWN     				*/KeyCode::KEY_VOLUME_DOWN,
/* AKEYCODE_POWER           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CAMERA          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CLEAR           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_A               				*/KeyCode::KEY_A,
/* AKEYCODE_B               				*/KeyCode::KEY_B,
/* AKEYCODE_C               				*/KeyCode::KEY_C,
/* AKEYCODE_D               				*/KeyCode::KEY_D,
/* AKEYCODE_E               				*/KeyCode::KEY_E,
/* AKEYCODE_F               				*/KeyCode::KEY_F,
/* AKEYCODE_G               				*/KeyCode::KEY_G,
/* AKEYCODE_H               				*/KeyCode::KEY_H,
/* AKEYCODE_I               				*/KeyCode::KEY_I,
/* AKEYCODE_J               				*/KeyCode::KEY_J,
/* AKEYCODE_K               				*/KeyCode::KEY_K,
/* AKEYCODE_L               				*/KeyCode::KEY_L,
/* AKEYCODE_M               				*/KeyCode::KEY_M,
/* AKEYCODE_N               				*/KeyCode::KEY_N,
/* AKEYCODE_O               				*/KeyCode::KEY_O,
/* AKEYCODE_P               				*/KeyCode::KEY_P,
/* AKEYCODE_Q               				*/KeyCode::KEY_Q,
/* AKEYCODE_R               				*/KeyCode::KEY_R,
/* AKEYCODE_S               				*/KeyCode::KEY_S,
/* AKEYCODE_T               				*/KeyCode::KEY_T,
/* AKEYCODE_U               				*/KeyCode::KEY_U,
/* AKEYCODE_V               				*/KeyCode::KEY_V,
/* AKEYCODE_W               				*/KeyCode::KEY_W,
/* AKEYCODE_X               				*/KeyCode::KEY_X,
/* AKEYCODE_Y               				*/KeyCode::KEY_Y,
/* AKEYCODE_Z               				*/KeyCode::KEY_Z,
/* AKEYCODE_COMMA           				*/KeyCode::KEY_COMMA,
/* AKEYCODE_PERIOD          				*/KeyCode::KEY_PERIOD,
/* AKEYCODE_ALT_LEFT        				*/KeyCode::KEY_LEFT_ALT,
/* AKEYCODE_ALT_RIGHT       				*/KeyCode::KEY_RIGHT_ALT,
/* AKEYCODE_SHIFT_LEFT      				*/KeyCode::KEY_LEFT_SHIFT,
/* AKEYCODE_SHIFT_RIGHT     				*/KeyCode::KEY_RIGHT_SHIFT,
/* AKEYCODE_TAB             				*/KeyCode::KEY_TAB,
/* AKEYCODE_SPACE           				*/KeyCode::KEY_SPACE,
/* AKEYCODE_SYM             				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_EXPLORER        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ENVELOPE        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ENTER           				*/KeyCode::KEY_ENTER,
/* AKEYCODE_DEL             				*/KeyCode::KEY_BACKSPACE,
/* AKEYCODE_GRAVE           				*/KeyCode::KEY_GRAVE,
/* AKEYCODE_MINUS           				*/KeyCode::KEY_MINUS,
/* AKEYCODE_EQUALS          				*/KeyCode::KEY_EQUALS,
/* AKEYCODE_LEFT_BRACKET    				*/KeyCode::KEY_LEFT_BRACKET,
/* AKEYCODE_RIGHT_BRACKET   				*/KeyCode::KEY_RIGHT_BRACKET,
/* AKEYCODE_BACKSLASH       				*/KeyCode::KEY_BACKSLASH,
/* AKEYCODE_SEMICOLON       				*/KeyCode::KEY_SEMICOLON,
/* AKEYCODE_APOSTROPHE      				*/KeyCode::KEY_APOSTROPHE,
/* AKEYCODE_SLASH           				*/KeyCode::KEY_SLASH,
/* AKEYCODE_AT              				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_NUM             				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_HEADSETHOOK     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_FOCUS           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PLUS            				*/KeyCode::KEY_PLUS,
/* AKEYCODE_MENU            				*/KeyCode::KEY_MENU,
/* AKEYCODE_NOTIFICATION    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SEARCH          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_PLAY_PAUSE				*/KeyCode::KEY_MEDIA_PLAY_PAUSE,
/* AKEYCODE_MEDIA_STOP      				*/KeyCode::KEY_MEDIA_STOP,
/* AKEYCODE_MEDIA_NEXT      				*/KeyCode::KEY_MEDIA_NEXT,
/* AKEYCODE_MEDIA_PREVIOUS  				*/KeyCode::KEY_MEDIA_PREVIOUS,
/* AKEYCODE_MEDIA_REWIND    				*/KeyCode::KEY_MEDIA_REWIND,
/* AKEYCODE_MEDIA_FAST_FORWARD 				*/KeyCode::KEY_MEDIA_FAST_FORWARD,
/* AKEYCODE_MUTE            				*/KeyCode::KEY_MIC_MUTE,
/* AKEYCODE_PAGE_UP         				*/KeyCode::KEY_PAGE_UP,
/* AKEYCODE_PAGE_DOWN       				*/KeyCode::KEY_PAGE_DOWN,
/* AKEYCODE_PICTSYMBOLS     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SWITCH_CHARSET  				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_A        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_B        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_C        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_X        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_Y        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_Z        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_L1       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_R1       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_L2       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_R2       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_THUMBL   				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_THUMBR   				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_START    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_SELECT   				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_MODE     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ESCAPE          				*/KeyCode::KEY_ESC,
/* AKEYCODE_FORWARD_DEL     				*/KeyCode::KEY_DELETE,
/* AKEYCODE_CTRL_LEFT       				*/KeyCode::KEY_LEFT_CTRL,
/* AKEYCODE_CTRL_RIGHT      				*/KeyCode::KEY_RIGHT_CTRL,
/* AKEYCODE_CAPS_LOCK       				*/KeyCode::KEY_CAPSLOCK,
/* AKEYCODE_SCROLL_LOCK     				*/KeyCode::KEY_SCROLLLOCK,
/* AKEYCODE_META_LEFT       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_META_RIGHT      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_FUNCTION        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SYSRQ           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BREAK           				*/KeyCode::KEY_BREAK,
/* AKEYCODE_MOVE_HOME       				*/KeyCode::KEY_HOME,
/* AKEYCODE_MOVE_END        				*/KeyCode::KEY_END,
/* AKEYCODE_INSERT          				*/KeyCode::KEY_INSERT,
/* AKEYCODE_FORWARD         				*/KeyCode::KEY_FORWARD,
/* AKEYCODE_MEDIA_PLAY      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_PAUSE     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_CLOSE     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_EJECT     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_RECORD    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_F1              				*/KeyCode::KEY_F1,
/* AKEYCODE_F2              				*/KeyCode::KEY_F2,
/* AKEYCODE_F3              				*/KeyCode::KEY_F3,
/* AKEYCODE_F4              				*/KeyCode::KEY_F4,
/* AKEYCODE_F5              				*/KeyCode::KEY_F5,
/* AKEYCODE_F6              				*/KeyCode::KEY_F6,
/* AKEYCODE_F7              				*/KeyCode::KEY_F7,
/* AKEYCODE_F8              				*/KeyCode::KEY_F8,
/* AKEYCODE_F9              				*/KeyCode::KEY_F9,
/* AKEYCODE_F10             				*/KeyCode::KEY_F10,
/* AKEYCODE_F11             				*/KeyCode::KEY_F11,
/* AKEYCODE_F12             				*/KeyCode::KEY_F12,
/* AKEYCODE_NUM_LOCK        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_NUMPAD_0        				*/KeyCode::KEY_NUM_0,
/* AKEYCODE_NUMPAD_1        				*/KeyCode::KEY_NUM_1,
/* AKEYCODE_NUMPAD_2        				*/KeyCode::KEY_NUM_2,
/* AKEYCODE_NUMPAD_3        				*/KeyCode::KEY_NUM_3,
/* AKEYCODE_NUMPAD_4        				*/KeyCode::KEY_NUM_4,
/* AKEYCODE_NUMPAD_5        				*/KeyCode::KEY_NUM_5,
/* AKEYCODE_NUMPAD_6        				*/KeyCode::KEY_NUM_6,
/* AKEYCODE_NUMPAD_7        				*/KeyCode::KEY_NUM_7,
/* AKEYCODE_NUMPAD_8        				*/KeyCode::KEY_NUM_8,
/* AKEYCODE_NUMPAD_9        				*/KeyCode::KEY_NUM_9,
/* AKEYCODE_NUMPAD_DIVIDE   				*/KeyCode::KEY_NUM_DIV,
/* AKEYCODE_NUMPAD_MULTIPLY 				*/KeyCode::KEY_NUM_MULT,
/* AKEYCODE_NUMPAD_SUBTRACT 				*/KeyCode::KEY_NUM_SUBTRACT,
/* AKEYCODE_NUMPAD_ADD      				*/KeyCode::KEY_NUM_ADD,
/* AKEYCODE_NUMPAD_DOT      				*/KeyCode::KEY_NUM_DOT,
/* AKEYCODE_NUMPAD_COMMA    				*/KeyCode::KEY_NUM_COMMA,
/* AKEYCODE_NUMPAD_ENTER    				*/KeyCode::KEY_NUM_ENTER,
/* AKEYCODE_NUMPAD_EQUALS   				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_NUMPAD_LEFT_PAREN 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_NUMPAD_RIGHT_PAREN 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_VOLUME_MUTE     				*/KeyCode::KEY_VOLUME_MUTE,
/* AKEYCODE_INFO            				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CHANNEL_UP      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CHANNEL_DOWN    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ZOOM_IN         				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ZOOM_OUT        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV              				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_WINDOW          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_GUIDE           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_DVR             				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BOOKMARK        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CAPTIONS        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SETTINGS        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_POWER        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_STB_POWER       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_STB_INPUT       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_AVR_POWER       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_AVR_INPUT       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PROG_RED        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PROG_GREEN      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PROG_YELLOW     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PROG_BLUE       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_APP_SWITCH      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_1        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_2        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_3        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_4        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_5        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_6        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_7        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_8        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_9        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_10       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_11       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_12       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_13       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_14       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_15       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BUTTON_16       				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_LANGUAGE_SWITCH 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MANNER_MODE     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_3D_MODE         				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CONTACTS        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CALENDAR        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MUSIC           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_CALCULATOR      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ZENKAKU_HANKAKU 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_EISU            				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MUHENKAN        				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_HENKAN          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_KATAKANA_HIRAGANA 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_YEN             				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_RO              				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_KANA            				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_ASSIST          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BRIGHTNESS_DOWN 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_BRIGHTNESS_UP   				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_AUDIO_TRACK 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_SLEEP           				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_WAKEUP          				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_PAIRING         				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_MEDIA_TOP_MENU  				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_11              				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_12              				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_LAST_CHANNEL    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_DATA_SERVICE 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_VOICE_ASSIST    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_RADIO_SERVICE 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_TELETEXT     				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_NUMBER_ENTRY 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_TERRESTRIAL_ANALOG 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_TERRESTRIAL_DIGITAL 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_SATELLITE    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_SATELLITE_BS 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_SATELLITE_CS 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_SATELLITE_SERVICE 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_NETWORK      				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_ANTENNA_CABLE 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_HDMI_1 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_HDMI_2 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_HDMI_3 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_HDMI_4 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_COMPOSITE_1 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_COMPOSITE_2 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_COMPONENT_1 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_COMPONENT_2 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_INPUT_VGA_1  				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_AUDIO_DESCRIPTION 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP 	*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN	*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_ZOOM_MODE    				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_CONTENTS_MENU 				*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_MEDIA_CONTEXT_MENU 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_TV_TIMER_PROGRAMMING 			*/KeyCode::KEY_UNKNOWN,
/* AKEYCODE_HELP            				*/KeyCode::KEY_UNKNOWN,

};

/**
 * As seen in android.view.KeyCharacterMap.VIRTUAL_KEYBOARD constant
 */
#define VIRTUAL_KEYBOARD_DEVICE_ID -1

static KeyModifiers getKeyModifiers(int32_t meta) {
	KeyModifiers modifiers = KeyModifiers::NO_FLAG;
	if (HAS_FLAG(meta, AMETA_ALT_LEFT_ON)) {
		modifiers |= KeyModifiers::LEFT_ALT;
	}
	if (HAS_FLAG(meta, AMETA_ALT_RIGHT_ON)) {
		modifiers |= KeyModifiers::RIGHT_ALT;
	}

	if (HAS_FLAG(meta, AMETA_SHIFT_LEFT_ON)) {
		modifiers |= KeyModifiers::LEFT_SHIFT;
	}
	if (HAS_FLAG(meta, AMETA_SHIFT_RIGHT_ON)) {
		modifiers |= KeyModifiers::RIGHT_SHIFT;
	}

#if __ANDROID_API__ >= 13
	if (HAS_FLAG(meta, AMETA_CTRL_LEFT_ON)) {
		modifiers |= KeyModifiers::LEFT_CTRL;
	}
	if (HAS_FLAG(meta, AMETA_CTRL_RIGHT_ON)) {
		modifiers |= KeyModifiers::RIGHT_CTRL;
	}

	if (HAS_FLAG(meta, AMETA_CAPS_LOCK_ON)) {
		modifiers |= KeyModifiers::CAPSLOCK;
	}
	if (HAS_FLAG(meta, AMETA_NUM_LOCK_ON)) {
		modifiers |= KeyModifiers::NUMLOCK;
	}
	if (HAS_FLAG(meta, AMETA_SCROLL_LOCK_ON)) {
		modifiers |= KeyModifiers::SCROLLLOCK;
	}
#endif
	return modifiers;
}

#define INPUT_HANDLED 1
#define INPUT_NOT_HANDLED 0

#define RETURN_HANDLE_INPUT(handled) AInputQueue_finishEvent(window->inputQueue, event, handled); return;

void platform_bridge::handleMotionInput(AInputEvent* event, core::AndroidWindow* window) {
	int32_t source = AInputEvent_getSource(event);
	int32_t deviceid = AInputEvent_getDeviceId(event);
	//this is java.lang.System.nanoTime() time base, which is specified as CLOCK_MONOTONIC via clock_gettime() in the documentation
	//convert to ms
	core::time_millis eventtime { AMotionEvent_getEventTime(event) / 1000000 };

	//LOGI("AINPUT_EVENT_TYPE_MOTION: source: 0x%X, deviceid: %d, time: %lld, contexttime: %lld", source, deviceid, eventtime, core::MonotonicTime::getCurrent());

	const int action = AMotionEvent_getAction(event);
	const int actionMasked = action & AMOTION_EVENT_ACTION_MASK;

	const InputDevice inputdevice =
#if __ANDROID_API__ >= 14
			HAS_FLAG(source, AINPUT_SOURCE_STYLUS) ? InputDevice::STYLUS :
#endif /* __ANDROID_API__ >= 14 */
			HAS_FLAG(source, AINPUT_SOURCE_MOUSE) ? InputDevice::MOUSE : InputDevice::FINGER;

	switch (actionMasked) {
		case AMOTION_EVENT_ACTION_DOWN: {
			const int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, pointerIndex), TouchAction::DOWN,
					AMotionEvent_getX(event, pointerIndex), AMotionEvent_getY(event, pointerIndex), eventtime);
			break;
		}
		case AMOTION_EVENT_ACTION_UP: {
			const int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, pointerIndex), TouchAction::UP,
					AMotionEvent_getX(event, pointerIndex), AMotionEvent_getY(event, pointerIndex), eventtime);
			break;
		}
		case AMOTION_EVENT_ACTION_POINTER_DOWN: {
			const int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, pointerIndex), TouchAction::DOWN,
					AMotionEvent_getX(event, pointerIndex), AMotionEvent_getY(event, pointerIndex), eventtime);
			break;
		}
		case AMOTION_EVENT_ACTION_POINTER_UP: {
			const int pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, pointerIndex), TouchAction::UP,
					AMotionEvent_getX(event, pointerIndex), AMotionEvent_getY(event, pointerIndex), eventtime);
			break;
		}
		case AMOTION_EVENT_ACTION_MOVE: {
			const int source = AInputEvent_getSource(event);
			const int pointerCount = AMotionEvent_getPointerCount(event);
			for (int i = 0; i < pointerCount; ++i) {
				TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, i), TouchAction::MOVE_UPDATE,
						AMotionEvent_getX(event, i), AMotionEvent_getY(event, i), eventtime);
			}
			TouchMessage::postMessage(window, inputdevice, -1, TouchAction::MOVE_UPDATE_DONE, 0.0f, 0.0f, eventtime);
			break;
		}
		case AMOTION_EVENT_ACTION_CANCEL: {
			TouchMessage::postMessage(window, inputdevice, -1, TouchAction::CANCEL, 0.0f, 0.0f, eventtime);
			break;
		}
#if __ANDROID_API__ >= 13
			case AMOTION_EVENT_ACTION_HOVER_MOVE: {
				TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, 0), TouchAction::HOVER_MOVE, AMotionEvent_getX(event, 0),
						AMotionEvent_getY(event, 0), eventtime);
				break;
			}
			case AMOTION_EVENT_ACTION_SCROLL: {
				LOGV() << "AMOTION_EVENT_ACTION_SCROLL";
				if (androidplatform::getSdkVersion() >= 13) {
					TouchEvent::ExtraData extra;

					extra.scrollVertical = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
					extra.scrollHorizontal = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, 0);
					TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, 0), TouchAction::SCROLL, AMotionEvent_getX(event, 0),
							AMotionEvent_getY(event, 0), eventtime, extra);
				}
				break;
			}
#if __ANDROID_API__ >= 14
			case AMOTION_EVENT_ACTION_HOVER_ENTER: {
				TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, 0), TouchAction::HOVER_ENTER, AMotionEvent_getX(event, 0),
						AMotionEvent_getY(event, 0), eventtime);
				break;
			}
			case AMOTION_EVENT_ACTION_HOVER_EXIT: {
				TouchMessage::postMessage(window, inputdevice, AMotionEvent_getPointerId(event, 0), TouchAction::HOVER_EXIT, AMotionEvent_getX(event, 0),
						AMotionEvent_getY(event, 0), eventtime);
				break;
			}
#endif /* __ANDROID_API__ >= 14 */
#endif /* __ANDROID_API__ >= 13 */
		default: {
			LOGI()<< "Unknown motion event action " << actionMasked;
			break;
		}
	}
	RETURN_HANDLE_INPUT(INPUT_HANDLED);
}

#define COMBINING_ACCENT 0x80000000

void platform_bridge::handleKeyInput(AInputEvent* event, core::AndroidWindow* window, int32_t keycode) {
	int32_t source = AInputEvent_getSource(event);
	int32_t deviceid = AInputEvent_getDeviceId(event);

	//this is java.lang.System.nanoTime() time base, which is specified as CLOCK_MONOTONIC via clock_gettime() in the documentation
	//convert to ms
	int64_t eventtime = AKeyEvent_getEventTime(event) / 1000000;

	LOGI()<< "AINPUT_EVENT_TYPE_KEY: source: " << source << ", deviceid: " << deviceid;
	int32_t action = AKeyEvent_getAction(event);
	int32_t flags = AKeyEvent_getFlags(event);
	int32_t metastate = AKeyEvent_getMetaState(event);

	const KeyModifiers modifiers = getKeyModifiers(metastate);

	const InputDevice inputdevice =
			(source == AINPUT_SOURCE_UNKNOWN) ? (InputDevice::UNKNOWN) : (
#if __ANDROID_API__ >= 14
												// XXX can a stylus have key event?
					HAS_FLAG(source, AINPUT_SOURCE_STYLUS) ? InputDevice::STYLUS :
#endif /* __ANDROID_API__ >= 14 */
					source == AINPUT_SOURCE_MOUSE ?
							InputDevice::MOUSE :
							(source == AINPUT_SOURCE_KEYBOARD ?
									(deviceid == VIRTUAL_KEYBOARD_DEVICE_ID ? InputDevice::VIRTUAL_KEYBOARD : InputDevice::KEYBOARD) :
									InputDevice::UNKNOWN));

	JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	//Initialized when exution gets here first
	UnicodeCodePoint unicode = (unsigned int) env.CallIntMethod(window->activity->clazz, getUnicodeFromKeyMethodId, keycode, metastate,
			deviceid);
	const bool accent = HAS_FLAG(unicode, COMBINING_ACCENT);
	if (accent) {
		unicode = unicode & ~COMBINING_ACCENT;
	}

	LOGV()<< "Keycode: " << keycode << ", mapped: " << KEYCODE_MAP[keycode] << " action: " << action;

	bool keymessagehandled = false;
	switch (action) {
		case AKEY_EVENT_ACTION_DOWN: {
			KeyMessage::ExtraData extra;
			extra.keycode = keycode >= (sizeof(KEYCODE_MAP) / sizeof(KEYCODE_MAP[0])) ? KeyCode::KEY_UNKNOWN : KEYCODE_MAP[keycode];
			extra.modifiers = modifiers;
			extra.repeat = AKeyEvent_getRepeatCount(event) > 0;
			keymessagehandled = KeyMessage::postMessage(window, inputdevice, KeyAction::DOWN, extra);
			if (unicode != 0) {
				//delivery key
				if (window->previousAccent != 0) {
					//combine
					UnicodeCodePoint combined = (unsigned int) env.CallStaticIntMethod(mainThreadState.KeyCharacterMapClass,
							mainThreadState.getDeadCharMethodId, window->previousAccent, unicode);
					extra.repeatUnicodeCount = 1;
					//XXX create new event action type for delivering combined, to override?
					if (combined == 0) {
						//failed to combine
						//deliver both
						extra.repeatUnicode = window->previousAccent;
						LOGI()<< "send unicode: " << (unsigned int )extra.repeatUnicode;
						keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UNICODE_REPEAT, extra);

						extra.repeatUnicode = unicode;
						LOGI()<< "send unicode: " << (unsigned int )extra.repeatUnicode;
						keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UNICODE_REPEAT, extra);
					} else {
						//successful combine
						extra.repeatUnicode = combined;
						LOGI()<< "send unicode: " << (unsigned int ) extra.repeatUnicode;
						keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UNICODE_REPEAT, extra);
					}
					window->previousAccent = (unsigned int) 0;
				} else if (accent) {
					//store accent
					window->previousAccent = unicode;
				} else {
					//simply deliver
					extra.repeatUnicode = unicode;
					extra.repeatUnicodeCount = 1;
					LOGI()<< "send unicode: " << (unsigned int )extra.repeatUnicode;
					keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UNICODE_REPEAT, extra);
				}
			}
			break;
		}
		case AKEY_EVENT_ACTION_UP: {
			KeyMessage::ExtraData extra;
			extra.keycode = KEYCODE_MAP[keycode];
			extra.modifiers = modifiers;
			keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UP, extra);
			break;
		}
		case AKEY_EVENT_ACTION_MULTIPLE: {
			LOGV()<< "AKEY_EVENT_ACTION_MULTIPLE";
			//wait for APP_CMD_MULTIPLE_UNICODE_INPUT later from dispatchKeyEvent
			RETURN_HANDLE_INPUT(INPUT_NOT_HANDLED);
			/*if (keycode == AKEYCODE_UNKNOWN) {
			 } else {
			 LOGI() << "repeat unicode char: " << (unsigned int)unicode;
			 KeyMessage::ExtraData extra;
			 extra.repeatUnicode = unicode;
			 extra.repeatUnicodeCount = AKeyEvent_getRepeatCount(event);
			 keymessagehandled |= KeyMessage::postMessage(window, inputdevice, KeyAction::UNICODE_REPEAT, extra);

			 RETURN_HANDLE_INPUT(INPUT_NOT_HANDLED);
			 }*/
			break;
		}
		default: {
			break;
		}
	}

	//TODO temporary
	if (!keymessagehandled) {
		switch (keycode) {
			case AKEYCODE_BACK: {
				if (action == AKEY_EVENT_ACTION_DOWN) {
					window->close();
				}
				break;
			}
			default: {
				RETURN_HANDLE_INPUT(INPUT_NOT_HANDLED);
				break;
			}
		}

	}
	RETURN_HANDLE_INPUT(INPUT_HANDLED);
}

static void handleMultipleUnicodeInput_native(JNIEnv* env, jobject actvity, jlong nativeptr, jobject buffer, jint count) {
	UnicodeCodePoint* codepoints = (UnicodeCodePoint*) env->GetDirectBufferAddress(buffer);

	static_assert(sizeof(UnicodeCodePoint) == 4, "Unicode char size is not 4");
	writeAppData(CommandMessage<UnicodeInputCommandMessage> { APP_CMD_MULTIPLE_UNICODE_INPUT, reinterpret_cast<ANativeActivity*>(nativeptr),
			codepoints, (unsigned int) count });
}

void platform_bridge::handleAppInput(AInputEvent* event, core::AndroidWindow* window) {
	switch (AInputEvent_getType(event)) {
		case AINPUT_EVENT_TYPE_MOTION: {
			platform_bridge::handleMotionInput(event, window);
			break;
		}
		case AINPUT_EVENT_TYPE_KEY: {
			int32_t keycode = AKeyEvent_getKeyCode(event);
			if (keycode != AKEYCODE_BACK) {
				//if keycode is not BACK, then give a chance to predispatch
				//if it is BACK, then the software keyboard would get closed without us getting notified
				if (AInputQueue_preDispatchEvent(window->inputQueue, event)) {
					return;
				}
			}
			platform_bridge::handleKeyInput(event, window, keycode);
			break;
		}
		default: {
			RETURN_HANDLE_INPUT(INPUT_NOT_HANDLED);
		}
	}
}

int platform_bridge::looperInputQueueFdCallback(int fd, int events, void* data) {
	core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(data);
	ASSERT(window != nullptr) << "Activity state for NativeActivity is nullptr";

	AInputEvent* event = nullptr;
	while (AInputQueue_getEvent(window->inputQueue, &event) >= 0) {
		/*if (AInputQueue_preDispatchEvent(window->inputQueue, event)) {
		 continue;
		 }*/
		handleAppInput(event, window);
	}
	return 1;
}

int platform_bridge::looperCommandFdCallback(int fd, int events, void* unused) {
	//during heavy restarting session fd might be invalid
	//debugnew might be causing this, hope its fixed
	ASSERT(fd == mainThreadState.getPipeReadFd()) << "Looper callback file descriptor is not valid: " << fd;

	EmptyCommandMessage cmdmessage;
	int readres = read(fd, &cmdmessage, sizeof(cmdmessage));
	if (readres == sizeof(cmdmessage)) {
		switch (cmdmessage.cmd) {
			case APP_CMD_INPUT_CHANGED: {
				LOGV()<< "APP_CMD_INPUT_CHANGED";

				InputChangedCommandMessage queuemsg;
				if (read(fd, &queuemsg, sizeof(queuemsg)) == sizeof(queuemsg)) {
					core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
					if (window->inputQueue != nullptr) {
						LOGV() << "Detaching input queue from looper " << window->inputQueue;
						AInputQueue_detachLooper(window->inputQueue);
					}
					window->inputQueue = queuemsg.queue;
					if (queuemsg.queue != nullptr) {
						LOGV() << "Attaching input queue to looper " << window->inputQueue;
						AInputQueue_attachLooper(queuemsg.queue, mainThreadState.looper, ALOOPER_POLL_CALLBACK,
								platform_bridge::looperInputQueueFdCallback, window);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_WINDOW_CREATED: {
				LOGV() << "APP_CMD_WINDOW_CREATED";

				WindowCommandMessage windowmsg;
				if (read(fd, &windowmsg, sizeof(windowmsg)) == sizeof(windowmsg)) {
					core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
					window->setNativeWindow(windowmsg.window, mainThreadState.getMonotonicTime());

				} else {
					THROW() << "No data on command pipe!";
				}

				break;
			}
			case APP_CMD_WINDOW_DESTROYING: {
				LOGV() << "APP_CMD_WINDOW_DESTROYING";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				window->setNativeWindow(nullptr, mainThreadState.getMonotonicTime());
				break;
			}
			case APP_CMD_WINDOW_RESIZED: {
				LOGV() << "APP_CMD_WINDOW_RESIZED";

				break;
			}
			case APP_CMD_WINDOW_REDRAW_NEEDED: {
				LOGV() << "APP_CMD_WINDOW_REDRAW_NEEDED";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				ASSERT(window->hasNativeWindow()) << "need redraw when not foreground || has no window";
				window->draw();
				break;
			}
			case APP_CMD_CONTENT_RECT_CHANGED: {
				LOGV() << "APP_CMD_CONTENT_RECT_CHANGED";
				//sometimes here we dont have a window yet
				ContentRectChangedCommandMessage rectmsg;
				if (read(fd, &rectmsg, sizeof(rectmsg)) == sizeof(rectmsg)) {
					core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
					window->setContentRect(rectmsg.rect.right - rectmsg.rect.left, rectmsg.rect.bottom - rectmsg.rect.top);

				} else {
					THROW() << "No data on command pipe!";
				}

				break;
			}
			case APP_CMD_GAINED_FOCUS: {
				LOGV() << "APP_CMD_GAINED_FOCUS";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				ASSERT(window->hasNativeWindow()) << "gained focus, but doesnt have window";

				//here app is not always foreground, because buggy Note 3, at first install the activity gets recreated
				window->setInputFocused(true);

				break;
			}
			case APP_CMD_LOST_FOCUS: {
				LOGV() << "APP_CMD_LOST_FOCUS";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				window->setInputFocused(false);

				break;
			}
			case APP_CMD_CONFIG_CHANGED: {
				LOGV() << "APP_CMD_CONFIG_CHANGED";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				AConfiguration* conf = AConfiguration_new();
				AConfiguration_fromAssetManager(conf, window->activity->assetManager);
				AConfiguration* oldconf = mainThreadState.configuration;

				handleConfigChanges(conf, AConfiguration_diff(oldconf, conf));
				AConfiguration_delete(oldconf);

				mainThreadState.configuration = conf;

				window->isLandscape = AConfiguration_getOrientation(mainThreadState.configuration) == ACONFIGURATION_ORIENTATION_LAND;
				//content rect change will come later, to update density
				break;
			}
			case APP_CMD_LOW_MEMORY: {
				LOGV() << "APP_CMD_LOW_MEMORY";
				break;
			}
			case APP_CMD_SAVE_STATE: {
				LOGV() << "APP_CMD_SAVE_STATE";
				break;
			}
			case APP_CMD_CREATE: {
				LOGV() << "APP_CMD_CREATE";

				ActivityCreateMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					mainThreadState.initialize();

					core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
					window = new core::AndroidWindow {cmdmessage.activity, msg.xdpi, msg.ydpi};
					core::Window::addWindow(*window, nullptr);	//TODO non-nullptr args
					cmdmessage.activity->instance = window;

					AConfiguration_fromAssetManager(mainThreadState.configuration, window->activity->assetManager);
					window->isLandscape = AConfiguration_getOrientation(mainThreadState.configuration) == ACONFIGURATION_ORIENTATION_LAND;
				} else {
					THROW() << "No data on command pipe!";
				}

				break;
			}
			case APP_CMD_START: {
				LOGV() << "APP_CMD_START";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				window->setVisibleToUser(true, mainThreadState.getMonotonicTime());
				break;
			}
			case APP_CMD_RESUME: {
				LOGV() << "APP_CMD_RESUME";
				break;
			}
			case APP_CMD_PAUSE: {
				LOGV() << "APP_CMD_PAUSE";
				break;
			}
			case APP_CMD_STOP: {
				LOGV() << "APP_CMD_STOP";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				window->setVisibleToUser(false, mainThreadState.getMonotonicTime());

				break;
			}
			case APP_CMD_DESTROY: {
				LOGV() << "APP_CMD_DESTROY";
				core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
				core::Window::removeWindow(*window);

				window->activity->instance = nullptr;
				delete window;

				break;
			}
			case APP_CMD_MULTIPLE_UNICODE_INPUT: {

				UnicodeInputCommandMessage unimsg;

				if (read(fd, &unimsg, sizeof(unimsg)) == sizeof(unimsg)) {
					for (unsigned int i = 0; i < unimsg.count; ++i) {
						LOGV() << "unicode char: " << (unsigned int )unimsg.codepoints[i];
					}
					KeyMessage::ExtraData extra;
					extra.sequenceUnicode = unimsg.codepoints;
					extra.sequenceUnicodeCount = unimsg.count;
					//TODO valid input device
					//TODO memory copy
					core::AndroidWindow* window = reinterpret_cast<core::AndroidWindow*>(cmdmessage.activity->instance);
					KeyMessage::postMessage(window, InputDevice::UNKNOWN, KeyAction::UNICODE_SEQUENCE, extra);
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_DISPLAY_ORIENTATION_CHANGED: {
				DisplayOrientationChangedMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					for (auto&& l : androidplatform::androidDeviceOrientationChangedEvents.objects()) {
						l.onAndroidDeviceOrientationChanged(msg.rotation);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_DRAW: {
				core::GlobalMonotonicTimeListener::setCurrent(mainThreadState.getMonotonicTime());
				executeDrawing();
				break;
			}
			default: {
				THROW() << "unknown command: " << (int ) cmdmessage.cmd;
				break;
			}
		}
		mainThreadState.commandCompletedSemaphore.post();
	} else {
		THROW()<< "No data on command pipe, read only: " << readres << " bytes, error: " << strerror(errno);
	}

//Implementations should return 1 to continue receiving callbacks, or 0
//to have this file descriptor and callback unregistered from the looper.
	return main_thread_closer.closed ? 0 : 1;
}

bool platform_bridge::executeDrawing() {

	bool result = false;
	for (auto&& w : core::Window::windows.objects()) {
		if (w.isReadyToDraw()) {
			w.draw();
			result = true;
		}
	}
	return result;
}
void platform_bridge::application_main(void* arg) {
	//somewhere there are memory leaks between activity restarts
	//https://code.google.com/p/android/issues/detail?id=201298
	LOGV()<< "Start application main thread";

	Thread::initApplicationMainThread();

	//initialize time, no listeners will be called, since user app, and modules are not initialized yet
	core::GlobalMonotonicTimeListener::setCurrent(mainThreadState.getMonotonicTime());

	//XXX use this block, to have Java side Looper available
	//however, it is still buggy?
	//https://code.google.com/p/android/issues/detail?id=229459
	// Java callbacks are not being called on this native looper
	/*
	 JNIEnv& env = *Thread::getJniEnvForCurrentThread();
	 jmethodID prep = env.GetStaticMethodID(androidplatform::nativeActivityClass, "createLooper", "()V");
	 env.CallStaticVoidMethod(androidplatform::nativeActivityClass, prep);
	 mainThreadState.looper = ALooper_forThread();
	 */
	mainThreadState.looper = ALooper_prepare(0);
	LOGTRACE() << "Looper: " << mainThreadState.looper;
	int res = ALooper_addFd(mainThreadState.looper, mainThreadState.getPipeReadFd(), ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
			looperCommandFdCallback, nullptr);
	ASSERT(res == 1) << "Failed to add file descriptor to looper";

	while (!main_thread_closer.closed) {
		core::GlobalMonotonicTimeListener::setCurrent(mainThreadState.getMonotonicTime());
		//TODO tracking waiting times
		const bool didDraw = false;//executeDrawing();

		const bool waitInfinite = !core::GlobalMonotonicTimeListener::hasListeners() && !didDraw;

		int pollres = ALooper_pollOnce(waitInfinite ? -1 : 0, nullptr, nullptr, nullptr);
		WARN(pollres != ALOOPER_POLL_WAKE && pollres != ALOOPER_POLL_CALLBACK && pollres != ALOOPER_POLL_TIMEOUT) << "Unknown looper return id: " << pollres;
		ASSERT(pollres != ALOOPER_POLL_ERROR) << "pollOnce returned ALOOPER_POLL_ERROR, " << strerror(errno);
	}

	mainThreadState.destroy();

}

static unsigned int internalPathLength(const char* p) {
	const char* ip = p;
	while (*ip != 0)
		++ip;
	if (*ip == FILE_SEPARATOR_CHAR)
		--ip;

	return ip - p;
}
static void draw_native(JNIEnv* env, jobject actvity, jlong nativeptr) {
	writeAppData(EmptyCommandMessage { APP_DRAW, reinterpret_cast<ANativeActivity*>(nativeptr) });
}
void platform_bridge::initializeFirstActivity(ANativeActivity* activity) {
	JNIEnv& env = *activity->env;
	jobject activityobj = activity->clazz;
	jclass activityclass = env.GetObjectClass(activityobj);

	bool first = Thread::setJavaVm(activity->vm);
	if (first) {
		androidplatform::nativeActivityClass = (jclass) env.NewGlobalRef(activityclass);

		androidplatform::sdkVersion = activity->sdkVersion;
		//we may retain the asset manager, as stated in it's documentation, it is an application instance:
		//Pointer to the Asset Manager instance for the application
		androidplatform::assetManager = activity->assetManager;

		LOGV()<< "Public file directory  = " << activity->externalDataPath;
		LOGV()<< "Private file directory = " << activity->internalDataPath;
		LOGV()<< "Android version number = " << activity->sdkVersion;

		StorageDirectoryDescriptor::initializeRootDirectory(
				FilePath { activity->internalDataPath, internalPathLength(activity->internalDataPath) } + "data");
		StorageDirectoryDescriptor { StorageDirectoryDescriptor::Root() }.create();

		LOGV()<< "Registering JNI methods";
		const JNINativeMethod methods[] = {

		{ "handleMultipleUnicodeInput", "(JLjava/nio/ByteBuffer;I)V", (void*) handleMultipleUnicodeInput_native },

		{ "nativeExecuteDraw", "(J)V", (void*) draw_native },

		};
		env.RegisterNatives(activityclass, methods, sizeof(methods) / sizeof(const JNINativeMethod));

		setNativePointerMethodId = env.GetMethodID(activityclass, "setNativePointer", "(J)V");
		ASSERT(setNativePointerMethodId != nullptr) << "Did not find method";
		getUnicodeFromKeyMethodId = env.GetMethodID(activityclass, "getUnicodeFromKey", "(III)I");
		ASSERT(getUnicodeFromKeyMethodId != nullptr) << "Did not find method";
		xdpiFieldId = env.GetFieldID(activityclass, "xdpi", "F");
		ASSERT(xdpiFieldId != nullptr) << "Did not find field";
		ydpiFieldId = env.GetFieldID(activityclass, "ydpi", "F");
		ASSERT(ydpiFieldId != nullptr) << "Did not find field";
	}

	jlong jptr = (jlong)(activity);
	env.CallVoidMethod(activityobj, setNativePointerMethodId, jptr);
	env.DeleteLocalRef(activityclass);

	if (first) {
		Thread _application_thread_instance { platform_bridge::application_main, nullptr };
	}
}

__attribute__ ((visibility ("default")))
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
	LOGV()<< "ANativeActivity_onCreate: " << activity << " tid: " << gettid();

	platform_bridge::initializeFirstActivity(activity);

	//START_TRACK_MEMORY_LEAKS();

	ANativeActivityCallbacks* callbacks = activity->callbacks;

	callbacks->onStart = onStart;
	callbacks->onResume = onResume;
	callbacks->onSaveInstanceState = onSaveInstanceState;
	callbacks->onPause = onPause;
	callbacks->onStop = onStop;
	callbacks->onDestroy = onDestroy;
	callbacks->onWindowFocusChanged = onWindowFocusChanged;
	callbacks->onNativeWindowCreated = onNativeWindowCreated;
	callbacks->onNativeWindowResized = onNativeWindowResized;
	callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
	callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
	callbacks->onInputQueueCreated = onInputQueueCreated;
	callbacks->onInputQueueDestroyed = onInputQueueDestroyed;
	callbacks->onContentRectChanged = onContentRectChanged;
	callbacks->onConfigurationChanged = onConfigurationChanged;
	callbacks->onLowMemory = onLowMemory;

	JNIEnv& env = *activity->env;
	jobject activityobj = activity->clazz;
	float xdpi = env.GetFloatField(activityobj, xdpiFieldId);
	float ydpi = env.GetFloatField(activityobj, ydpiFieldId);

	writeAppData(CommandMessage<ActivityCreateMessage> {APP_CMD_CREATE, activity, xdpi, ydpi});
}
