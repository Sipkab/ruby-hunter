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
 *  Created on: 2016. szept. 1.
 *      Author: sipka
 */

#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>
#include <framework/geometry/Vector.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/core/AppInterface.h>
#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/io/key/KeyModifierTracker.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/touch/TouchEvent.h>

#include <gen/log.h>
#include <gen/types.h>
#include <gen/configuration.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>

#include <linuxplatform/window/x11functions.h>
#include <linuxplatform/window/glxfunctions.h>
#include <linuxplatform/window/appglue.h>

#if RHFW_DEBUG
#include <link.h>
static const char* GetDlPath(void* dlhandle) {
	struct link_map* p;
	int res = dlinfo(dlhandle, RTLD_DI_LINKMAP, &p);
	return res == 0 ? p->l_name : "unknown path";
}
#else
#define GetDlPath(handle) ("")
#endif /* RHFW_DEBUG */

class ProgramArguments {
public:
	int argc;
	char** argv;
};

namespace rhfw {
class platform_bridge {
public:
	static Size2F dpi;

	static void application_main(void* arg);

	static bool executeDrawing();

	static bool processInputData();

	static void initDpi();

	static core::Window* getWindowForNativeWindow(::Window win) {
		for (auto&& w : core::Window::getWindows().objects()) {
			if (w.nativeWindow->window == win) {
				return &w;
			}
		}
		return nullptr;
	}
};
Size2F platform_bridge::dpi { 0, 0 };

} // namespace rhfw

class MainThreadState {
	int msgpipe[2] { 0, 0 };
public:
	rhfw::Semaphore commandCompletedSemaphore;

	MainThreadState() {
		commandCompletedSemaphore.init();

		int res;

		res = pipe(msgpipe);
		ASSERT(res == 0) << "Failed to create pipe: " << strerror(errno);

		int flags = fcntl(getPipeReadFd(), F_GETFL, 0);
		res = fcntl(getPipeReadFd(), F_SETFL, flags | O_NONBLOCK);
		ASSERT(res == 0) << "Failed to set nonblocking pipe: " << strerror(errno);
	}
	~MainThreadState() {
		int res;
		res = close(msgpipe[0]);
		ASSERT(res == 0) << "Failed to close pipe: " << strerror(errno);
		res = close(msgpipe[1]);
		ASSERT(res == 0) << "Failed to close pipe: " << strerror(errno);
	}

	int getPipeReadFd() const {
		return msgpipe[0];
	}
	int getPipeWriteFd() const {
		return msgpipe[1];
	}

	template<bool wait, typename T>
	void writeAppData(const T& data) {
		if (write(this->getPipeWriteFd(), &data, sizeof(T)) != sizeof(T)) {
			LOGE()<<"Failure writing app_cmd: " << strerror(errno);
			THROW() <<"Failed to write app data";
		}
		if(wait) {
			this->commandCompletedSemaphore.wait();
		}
	}
};
static MainThreadState mainThreadState;

static rhfw::core::time_micros getMonotonicTime() {
	struct timespec currentTime { 0 };
	int res = clock_gettime(CLOCK_MONOTONIC, &currentTime);
	ASSERT(res == 0) << "failed to get time error: " << strerror(errno);
	return rhfw::core::time_micros { static_cast<long long>(currentTime.tv_sec) * 1000000
			+ static_cast<long long>(currentTime.tv_nsec) / 1000 };
}
enum {
	APP_CMD_WINDOW_CREATED,
	APP_CMD_WINDOW_DESTROYED,
	APP_CMD_WINDOW_STATE_CHANGED,

	APP_CMD_DRAW,

	APP_CMD_MOTIONEVENT,
	APP_CMD_MOTIONEVENT_EXTRA,
	APP_CMD_KEYEVENT,

	APP_CMD_UNICODE_CHAR,

	APP_CMD_TERMINATE,
};
class __attribute((aligned(sizeof(void*)))) EmptyCommandMessage {
public:
	unsigned char cmd;
};
template<typename Messagetype>
class __attribute((aligned(sizeof(void*)))) CommandMessage {
public:
	EmptyCommandMessage msg;
	Messagetype data;
};
class __attribute((aligned(sizeof(void*)))) WindowCommandMessage {
public:
	Window window;
	XVisualInfo* visualInfo;
	Colormap colorMap;
};
class __attribute((aligned(sizeof(void*)))) MotionCommandMessage {
public:
	Window window;
	rhfw::InputDevice device;
	int touchid;
	rhfw::TouchAction action;
	float x;
	float y;
	rhfw::core::time_micros millis;
};
class __attribute((aligned(sizeof(void*)))) MotionExtraCommandMessage {
public:
	Window window;
	rhfw::InputDevice device;
	int touchid;
	rhfw::TouchAction action;
	float x;
	float y;
	rhfw::core::time_micros millis;
	rhfw::TouchEvent::ExtraData extra;
};
class __attribute((aligned(sizeof(void*)))) KeyCommandMessage {
public:
	Window window;
	rhfw::InputDevice device;
	rhfw::KeyAction action;
	rhfw::KeyMessage::ExtraData extra;
	rhfw::core::time_micros millis;
};
class __attribute((aligned(sizeof(void*)))) WindowStateCommandMessage {
public:
	Window window;
	int visible;
	unsigned int width;
	unsigned int height;
};
class __attribute((aligned(sizeof(void*)))) UnicodeCharMessage {
public:
	Window window;
	rhfw::UnicodeCodePoint codepoint;
};
class __attribute((aligned(sizeof(void*)))) DrawCommandMessage {
public:
	bool* resultout;
};
class __attribute((aligned(sizeof(void*)))) WindowDestroyCommandMessage {
public:
	Window window;
	bool* exitappout;
};
bool ::rhfw::platform_bridge::processInputData() {
	int fd = mainThreadState.getPipeReadFd();
	bool result = true;
	EmptyCommandMessage cmdmessage;
	size_t readres;
	while (result && (readres = read(fd, &cmdmessage, sizeof(cmdmessage))) == sizeof(cmdmessage)) {
		switch (cmdmessage.cmd) {
			case APP_CMD_WINDOW_CREATED: {
				WindowCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = new core::Window();
					w->nativeWindow->window = msg.window;
					w->nativeWindow->visualInfo = x11server::VisualInfoTracker::make(msg.visualInfo);
					w->nativeWindow->colorMap = msg.colorMap;

					core::Window::addWindow(*w, nullptr);
				} else {
					THROW()<< "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_DESTROYED: {
				WindowDestroyCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);

					if (w != nullptr) {
						core::Window::removeWindow(*w);
						delete w;
					}

					*msg.exitappout = !core::Window::hasWindows();
				} else {
					THROW()<< "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_STATE_CHANGED: {
				WindowStateCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);

					if (w != nullptr) {
						if (msg.width > 0) {
							w->setSize(core::WindowSize { Size2UI { msg.width, msg.height }, dpi });
						}
						if (msg.visible >= 0) {
							LOGI()<< "Window visibility changed: " << (msg.visible != 0);
							w->setVisibleToUser(msg.visible != 0, getMonotonicTime());
						}
					}
				} else {
					THROW()<< "No data on command pipe!";
				}
				//mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_MOTIONEVENT: {
				MotionCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);

					if (w != nullptr) {
						rhfw::TouchMessage::postMessage(w, msg.device, msg.touchid, msg.action, msg.x, msg.y, msg.millis);
					}
				} else {
					THROW()<< "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_MOTIONEVENT_EXTRA: {
				MotionExtraCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);

					if (w != nullptr) {
						rhfw::TouchMessage::postMessage(w, msg.device, msg.touchid, msg.action, msg.x, msg.y, msg.millis, msg.extra);
					}
				} else {
					THROW()<< "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_KEYEVENT: {
				KeyCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);
					//TODO do not ignore millis here
					if (w != nullptr) {
						rhfw::KeyMessage::postMessage(w, msg.device, msg.action, msg.extra);
					}
				} else {
					THROW()<< "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_UNICODE_CHAR: {
				UnicodeCharMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window* w = getWindowForNativeWindow(msg.window);

					if (w != nullptr) {
						KeyMessage::ExtraData extra;
						extra.repeatUnicode = msg.codepoint;
						extra.repeatUnicodeCount = 1;
						KeyMessage::postMessage(w, InputDevice::KEYBOARD, KeyAction::UNICODE_REPEAT, extra);
					}
				} else {
					THROW()<< "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_DRAW: {
				DrawCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					*msg.resultout = executeDrawing();
				} else {
					THROW()<< "No data on command pipe!";
				}

				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_TERMINATE: {
				LOGI()<< "APP_CMD_TERMINATE";
				result = false;
				user_app_terminate();
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			default: {
				THROW()<< "Invalid command message: " << (unsigned int)cmdmessage.cmd << " - " << cmdmessage.cmd;
				break;
			}
		}
	}
	ASSERT(readres == sizeof(cmdmessage) || errno == EAGAIN) << "No data on command pipe!, read: " << readres << " errno: "
			<< strerror(errno);
	return result;
}

bool ::rhfw::platform_bridge::executeDrawing() {
	bool result = false;
	for (auto&& w : core::Window::windows.objects()) {
		if (w.isReadyToDraw()) {
			w.draw();
			result = true;
		}
	}
	return result;
}

void ::rhfw::platform_bridge::application_main(void* arg) {
	ProgramArguments* pa = reinterpret_cast<ProgramArguments*>(arg);

	StorageDirectoryDescriptor::InitializePlatformRootDirectory(pa->argc, pa->argv);
	Thread::initApplicationMainThread();

	user_app_initialize(pa->argc, pa->argv);
	delete pa;

	int res;
	struct pollfd polldata;
	polldata.fd = mainThreadState.getPipeReadFd();
	polldata.events = POLLIN;

	while (true) {
		core::GlobalMonotonicTimeListener::setCurrent(getMonotonicTime());
		bool drawres = executeDrawing();

		const bool waitInfinite = !drawres && !core::GlobalMonotonicTimeListener::hasListeners();

		res = poll(&polldata, 1, waitInfinite ? -1 : 0);
		ASSERT(res >= 0) << "poll call failed: " << strerror(errno);

		if (res > 0) {
			//process data
			if (!processInputData()) {
				break;
			}
		}
	}
}

static void* libX11Handle = nullptr;
static void* libOpenGLHandle = nullptr;

static Display* x11MainDisplay = nullptr;
static Atom x11wmDeleteMessage = None;
static Atom x11wmState = None;
static Atom x11wmStateFullscreen = None;
static int notifyX11FileDescriptor = -1;

#define DECLARE_FUNCTION(prefix, name)

#define INSTANTIATE_GLXFUNC(name) PROTO_##name glxfunc_##name = nullptr
#define LOAD_GLXFUNC(name) loadGLXFunction(glxfunc_##name, STRINGIZE(name))
#define QUERY_GLXFUNC(name) queryGLXFunction(glxfunc_##name, STRINGIZE(name));

#define INSTANTIATE_XFUNC(name) PROTO_##name xfunc_##name = nullptr
#define LOAD_XFUNC(name) loadXFunction(xfunc_##name, STRINGIZE(name))

INSTANTIATE_XFUNC(XOpenDisplay);
INSTANTIATE_XFUNC(XDefaultRootWindow);
INSTANTIATE_XFUNC(XCreateColormap);
INSTANTIATE_XFUNC(XCreateWindow);
INSTANTIATE_XFUNC(XNextEvent);
INSTANTIATE_XFUNC(XEventsQueued);
INSTANTIATE_XFUNC(XSendEvent);
INSTANTIATE_XFUNC(XPending);
INSTANTIATE_XFUNC(XDestroyWindow);
INSTANTIATE_XFUNC(XCloseDisplay);
INSTANTIATE_XFUNC(XMapWindow);
INSTANTIATE_XFUNC(XStoreName);
INSTANTIATE_XFUNC(XCreatePixmap);
INSTANTIATE_XFUNC(XFreePixmap);
INSTANTIATE_XFUNC(XInitThreads);
INSTANTIATE_XFUNC(XLockDisplay);
INSTANTIATE_XFUNC(XUnlockDisplay);
INSTANTIATE_XFUNC(XKeycodeToKeysym);
INSTANTIATE_XFUNC(XLookupString);
INSTANTIATE_XFUNC(XFilterEvent);
INSTANTIATE_XFUNC(XFree);
INSTANTIATE_XFUNC(XInternAtom);
INSTANTIATE_XFUNC(XSetWMProtocols);
INSTANTIATE_XFUNC(XDisplayWidthMM);
INSTANTIATE_XFUNC(XDisplayHeightMM);
INSTANTIATE_XFUNC(XDisplayWidth);
INSTANTIATE_XFUNC(XDisplayHeight);
INSTANTIATE_XFUNC(XDefaultScreen);
INSTANTIATE_XFUNC(XConnectionNumber);
INSTANTIATE_XFUNC(XFreeColormap);
INSTANTIATE_XFUNC(XFlush);
INSTANTIATE_XFUNC(XSync);
INSTANTIATE_XFUNC(XSetErrorHandler);
INSTANTIATE_XFUNC(XDefineCursor);
INSTANTIATE_XFUNC(XCreateBitmapFromData);
INSTANTIATE_XFUNC(XFreeCursor);
INSTANTIATE_XFUNC(XCreatePixmapCursor);

INSTANTIATE_GLXFUNC(glXGetProcAddress);
INSTANTIATE_GLXFUNC(glXChooseVisual);
INSTANTIATE_GLXFUNC(glXMakeCurrent);
INSTANTIATE_GLXFUNC(glXCreateContext);
INSTANTIATE_GLXFUNC(glXDestroyContext);
INSTANTIATE_GLXFUNC(glXSwapBuffers);
INSTANTIATE_GLXFUNC(glXQueryExtensionsString);
INSTANTIATE_GLXFUNC(glXChooseFBConfig);
INSTANTIATE_GLXFUNC(glXGetVisualFromFBConfig);
INSTANTIATE_GLXFUNC(glXCreateGLXPixmap);
INSTANTIATE_GLXFUNC(glXDestroyGLXPixmap);

template<typename T>
inline static void loadDynamicFunction(void* dlhandle, T*& func, const char* name) {
	func = (T*) dlsym(dlhandle, name);
	if (func == nullptr) {
		printf("Function %s not found\n", name);
		exit(-2);
	}
}

template<typename T>
inline static void loadXFunction(T*& func, const char* name) {
	loadDynamicFunction(libX11Handle, func, name);
}
template<typename T>
inline static void loadGLXFunction(T*& func, const char* name) {
	loadDynamicFunction(libOpenGLHandle, func, name);
}

template<typename T>
inline static void queryGLXFunction(T*& func, const char* name) {
	func = (T*) glxfunc_glXGetProcAddress((const GLubyte*) name);
	if (func == nullptr) {
		printf("Function %s not found\n", name);
		exit(-3);
	}
}

#define FATAL_ERROR(message) printf(message); fprintf(stderr, message); exit(-1);

static void initX11() {
	libX11Handle = dlopen("libX11.so.6.3.0", RTLD_LAZY);
	if (libX11Handle == nullptr) {
		LOGE()<< "libX11.so.6.3.0 not found";
		libX11Handle = dlopen("libX11.so.6", RTLD_LAZY);
		if (libX11Handle == nullptr) {
			LOGE()<< "libX11.so.6 not found";
			libX11Handle = dlopen("libX11.so", RTLD_LAZY);
			if (libX11Handle == nullptr) {
				LOGE()<< "libX11.so not found";
				FATAL_ERROR("libX11 not found. Please make sure X11 server is installed and running.\n");
			}
		}
	}
	LOGI()<< "X11 lib: " << GetDlPath(libX11Handle);
	LOAD_XFUNC(XOpenDisplay);
	LOAD_XFUNC(XDefaultRootWindow);
	LOAD_XFUNC(XCreateColormap);
	LOAD_XFUNC(XCreateWindow);
	LOAD_XFUNC(XNextEvent);
	LOAD_XFUNC(XEventsQueued);
	LOAD_XFUNC(XSendEvent);
	LOAD_XFUNC(XPending);
	LOAD_XFUNC(XDestroyWindow);
	LOAD_XFUNC(XCloseDisplay);
	LOAD_XFUNC(XMapWindow);
	LOAD_XFUNC(XStoreName);
	LOAD_XFUNC(XCreatePixmap);
	LOAD_XFUNC(XFreePixmap);
	LOAD_XFUNC(XInitThreads);
	LOAD_XFUNC(XLockDisplay);
	LOAD_XFUNC(XUnlockDisplay);
	LOAD_XFUNC(XKeycodeToKeysym);
	LOAD_XFUNC(XLookupString);
	LOAD_XFUNC(XFilterEvent);
	LOAD_XFUNC(XFree);
	LOAD_XFUNC(XInternAtom);
	LOAD_XFUNC(XSetWMProtocols);
	LOAD_XFUNC(XDisplayWidthMM);
	LOAD_XFUNC(XDisplayHeightMM);
	LOAD_XFUNC(XDisplayWidth);
	LOAD_XFUNC(XDisplayHeight);
	LOAD_XFUNC(XDefaultScreen);
	LOAD_XFUNC(XConnectionNumber);
	LOAD_XFUNC(XFreeColormap);
	LOAD_XFUNC(XFlush);
	LOAD_XFUNC(XSync);
	LOAD_XFUNC(XSetErrorHandler);
	LOAD_XFUNC(XDefineCursor);
	LOAD_XFUNC(XCreateBitmapFromData);
	LOAD_XFUNC(XFreeCursor);
	LOAD_XFUNC(XCreatePixmapCursor);
}

static void initGLX() {
	libOpenGLHandle = dlopen("libGL.so", RTLD_LAZY);
	if (libOpenGLHandle == nullptr) {
		LOGE()<< "libGL.so not found";
		libOpenGLHandle = dlopen("libGL.so.1", RTLD_LAZY);
		if (libOpenGLHandle == nullptr) {
			LOGE() << "libGL.so.1 not found";
			libOpenGLHandle = dlopen("libGL.so.1.2.0", RTLD_LAZY);
			if (libOpenGLHandle == nullptr) {
				LOGE() << "libGL.so.1.2.0 not found";
				FATAL_ERROR("libGL not found. Please make sure OpenGL is installed.\n");
			}
		}
	}
	LOGI()<< "GL lib: " << GetDlPath(libOpenGLHandle);

	LOAD_GLXFUNC(glXGetProcAddress);
	if (glxfunc_glXGetProcAddress == nullptr) {
		loadGLXFunction(glxfunc_glXGetProcAddress, "glXGetProcAddressARB");
		if (glxfunc_glXGetProcAddress == nullptr) {
			printf("Function %s not found\n", "glXGetProcAddress");
			exit(-3);
		}
	}

	QUERY_GLXFUNC(glXChooseVisual);
	QUERY_GLXFUNC(glXMakeCurrent);
	QUERY_GLXFUNC(glXCreateContext);
	QUERY_GLXFUNC(glXDestroyContext);
	QUERY_GLXFUNC(glXSwapBuffers);
	QUERY_GLXFUNC(glXQueryExtensionsString);
	QUERY_GLXFUNC(glXChooseFBConfig);
	QUERY_GLXFUNC(glXGetVisualFromFBConfig);
	QUERY_GLXFUNC(glXCreateGLXPixmap);
	QUERY_GLXFUNC(glXDestroyGLXPixmap);
}

namespace rhfw {
namespace x11server {
Display* getMainDisplay() {
	return x11MainDisplay;
}
Atom getWmDeleteMessageAtom() {
	return x11wmDeleteMessage;
}
Atom getWmStateFullscreenAtom() {
	return x11wmStateFullscreen;
}
Atom getWmStateAtom() {
	return x11wmState;
}
void notifyX11Loop() {
	uint64 val = 1;
	write(notifyX11FileDescriptor, &val, 8);
}
}  // namespace x11server
}  // namespace rhfw

void ::rhfw::platform_bridge::initDpi() {
	auto widthmm = xfunc_XDisplayWidthMM(x11MainDisplay, 0);
	auto heightmm = xfunc_XDisplayHeightMM(x11MainDisplay, 0);
	auto widthpx = xfunc_XDisplayWidth(x11MainDisplay, 0);
	auto heightpx = xfunc_XDisplayHeight(x11MainDisplay, 0);
	if (widthmm <= 0 || heightmm <= 0) {
		dpi = {90, 90};
	} else {
		dpi = {(float)widthpx / widthmm * 25.4f, (float)heightpx / heightmm * 25.4f};
	}
	XDEBUGSYNC(x11MainDisplay);
}

static void createFirstWindow() {
	LOGTRACE()<< "Creating window";
	xfunc_XLockDisplay(x11MainDisplay);

	static GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	XVisualInfo* visualinfo = glxfunc_glXChooseVisual(x11MainDisplay, 0, att);
	ASSERT(visualinfo != nullptr);
	XDEBUGSYNC(x11MainDisplay);

	Window rootwindow = xfunc_XDefaultRootWindow(x11MainDisplay);
	XDEBUGSYNC(x11MainDisplay);

	XSetWindowAttributes swa {0};
	swa.colormap = xfunc_XCreateColormap(x11MainDisplay, rootwindow, visualinfo->visual, AllocNone);
	swa.event_mask = X11_WINDOW_EVENT_MASKS;
	XDEBUGSYNC(x11MainDisplay);

	Window win = xfunc_XCreateWindow(x11MainDisplay, rootwindow, 0, 0, 600, 600, 0, visualinfo->depth, InputOutput, visualinfo->visual,
			CWColormap | CWEventMask, &swa);
	XDEBUGSYNC(x11MainDisplay);

	xfunc_XSetWMProtocols(x11MainDisplay, win, &x11wmDeleteMessage, 1);
	xfunc_XStoreName(x11MainDisplay, win, APPLICATION_DEFAULT_NAME);
	XDEBUGSYNC(x11MainDisplay);

	xfunc_XMapWindow(x11MainDisplay, win);
	XDEBUGSYNC(x11MainDisplay);

	xfunc_XUnlockDisplay(x11MainDisplay);

	mainThreadState.writeAppData<true>(CommandMessage<WindowCommandMessage> {APP_CMD_WINDOW_CREATED, win, visualinfo, swa.colormap});
	XDEBUGSYNC(x11MainDisplay);
}

static void postUnicodeEvent(Window window, KeySym keysym) {
	rhfw::UnicodeCodePoint cp = 0;
	if ((unsigned int) keysym <= 0xFF) {
		//the XK_* - Unicode mapping are equals for the first 255 characters
		cp = (unsigned int) keysym;
	} else {
		//TODO handle the rest somehow
	}
	mainThreadState.writeAppData<false>(CommandMessage<UnicodeCharMessage> { APP_CMD_UNICODE_CHAR, window, cp });
}

static rhfw::KeyModifierTracker modifier_states;

template<rhfw::KeyAction ACTION, bool REPEAT = false>
static void postKeyEvent(Window window, KeySym keysym) {
	if (keysym == NoSymbol) {
		return;
	}
	rhfw::KeyMessage::ExtraData extra;
	extra.modifiers = modifier_states;
	extra.repeat = REPEAT;
	if (keysym >= XK_0 && keysym <= XK_9) {
		extra.keycode = (rhfw::KeyCode) (((unsigned int) rhfw::KeyCode::KEY_0) + (keysym - XK_0));
	} else if (keysym >= XK_KP_0 && keysym <= XK_KP_9) {
		extra.keycode = (rhfw::KeyCode) (((unsigned int) rhfw::KeyCode::KEY_NUM_0) + (keysym - XK_KP_0));
	} else if (keysym >= XK_A && keysym <= XK_Z) {
		extra.keycode = (rhfw::KeyCode) (((unsigned int) rhfw::KeyCode::KEY_A) + (keysym - XK_A));
	} else if (keysym >= XK_a && keysym <= XK_z) {
		extra.keycode = (rhfw::KeyCode) (((unsigned int) rhfw::KeyCode::KEY_A) + (keysym - XK_a));
	} else if (keysym >= XK_F1 && keysym <= XK_F12) {
		extra.keycode = (rhfw::KeyCode) (((unsigned int) rhfw::KeyCode::KEY_F1) + (keysym - XK_F1));
	} else {
		switch (keysym) {
#define MAP_KEY(key_, keycode_) case key_: extra.keycode = rhfw::KeyCode::keycode_; break;
			MAP_KEY(XK_BackSpace, KEY_BACKSPACE)
			MAP_KEY(XK_space, KEY_SPACE)
			MAP_KEY(XK_Escape, KEY_ESC)
			MAP_KEY(XK_Return, KEY_ENTER)
			MAP_KEY(XK_Tab, KEY_TAB)
			MAP_KEY(XK_Home, KEY_HOME)
			MAP_KEY(XK_End, KEY_END)
			MAP_KEY(XK_Delete, KEY_DELETE)
			MAP_KEY(XK_Insert, KEY_INSERT)
			MAP_KEY(XK_Left, KEY_DIR_LEFT)
			MAP_KEY(XK_Right, KEY_DIR_RIGHT)
			MAP_KEY(XK_Up, KEY_DIR_UP)
			MAP_KEY(XK_Down, KEY_DIR_DOWN)
			MAP_KEY(XK_Page_Up, KEY_PAGE_UP)
			MAP_KEY(XK_Page_Down, KEY_PAGE_DOWN)
			MAP_KEY(XK_Control_L, KEY_LEFT_CTRL)
			MAP_KEY(XK_Control_R, KEY_RIGHT_CTRL)
			MAP_KEY(XK_Shift_L, KEY_LEFT_SHIFT)
			MAP_KEY(XK_Shift_R, KEY_RIGHT_SHIFT)
			MAP_KEY(XK_Alt_L, KEY_LEFT_ALT)
			MAP_KEY(XK_Alt_R, KEY_RIGHT_ALT)
			MAP_KEY(XK_Menu, KEY_MENU)
			MAP_KEY(XK_Caps_Lock, KEY_CAPSLOCK)
			MAP_KEY(XK_Scroll_Lock, KEY_SCROLLLOCK)
			MAP_KEY(XK_Num_Lock, KEY_NUMLOCK)

			MAP_KEY(XK_KP_Separator, KEY_NUM_DOT)
			MAP_KEY(XK_KP_Divide, KEY_NUM_DIV)
			MAP_KEY(XK_KP_Multiply, KEY_NUM_MULT)
			MAP_KEY(XK_KP_Subtract, KEY_NUM_SUBTRACT)
			MAP_KEY(XK_KP_Add, KEY_NUM_ADD)
			MAP_KEY(XK_KP_Enter, KEY_NUM_ENTER)

			MAP_KEY(XK_KP_Insert, KEY_INSERT)
			MAP_KEY(XK_KP_Delete, KEY_DELETE)
			MAP_KEY(XK_KP_Next, KEY_PAGE_DOWN)
			MAP_KEY(XK_KP_Prior, KEY_PAGE_UP)
			MAP_KEY(XK_KP_Left, KEY_DIR_LEFT)
			MAP_KEY(XK_KP_Right, KEY_DIR_RIGHT)
			MAP_KEY(XK_KP_Up, KEY_DIR_UP)
			MAP_KEY(XK_KP_Down, KEY_DIR_DOWN)
			MAP_KEY(XK_KP_Home, KEY_HOME)
			MAP_KEY(XK_KP_End, KEY_END)
				//TODO Map numpad 5 to something
			MAP_KEY(XK_KP_Begin, KEY_CLEAR)

			MAP_KEY(XK_Pause, KEY_PAUSEBREAK)
			default: {
				extra.keycode = rhfw::KeyCode::KEY_UNKNOWN;
				break;
			}
		}
	}
	LOGTRACE()<< ACTION << ": " << extra.keycode;
	if (ACTION == rhfw::KeyAction::DOWN) {
		modifier_states.down(extra.keycode);
	} else if (ACTION == rhfw::KeyAction::UP) {
		modifier_states.up(extra.keycode);
	}
	//TODO track modifiers
	mainThreadState.writeAppData<false>(CommandMessage<KeyCommandMessage> { APP_CMD_KEYEVENT, window, rhfw::InputDevice::KEYBOARD, ACTION,
			extra, getMonotonicTime() });
}

int main(int argc, char *argv[]) {
	using namespace rhfw;

	int notifyeventfd = eventfd(0, 0);
	if (notifyeventfd < 0) {
		FATAL_ERROR("Failed to create notification file descriptor");
	}
	notifyX11FileDescriptor = notifyeventfd;

	initX11();

	xfunc_XInitThreads();

	initGLX();

	x11MainDisplay = xfunc_XOpenDisplay(nullptr);
	if (x11MainDisplay == nullptr) {
		FATAL_ERROR("Failed to open main display");
	}

	::rhfw::platform_bridge::initDpi();

	x11wmDeleteMessage = xfunc_XInternAtom(x11MainDisplay, "WM_DELETE_WINDOW", False);
	XDEBUGSYNC(x11MainDisplay);
	x11wmState = xfunc_XInternAtom(x11MainDisplay, "_NET_WM_STATE", False);
	XDEBUGSYNC(x11MainDisplay);
	x11wmStateFullscreen = xfunc_XInternAtom(x11MainDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	XDEBUGSYNC(x11MainDisplay);

	int connectionfd = xfunc_XConnectionNumber(x11MainDisplay);

	::rhfw::Thread main_thread { ::rhfw::platform_bridge::application_main, new ProgramArguments { argc, argv } };

	createFirstWindow();

	struct pollfd polldata[2];
	polldata[0].fd = connectionfd;
	polldata[0].events = POLLIN;

	polldata[1].fd = notifyeventfd;
	polldata[1].events = POLLIN;

	XComposeStatus compose;
	xfunc_XLockDisplay(x11MainDisplay);
	while (true) {
		XEvent xevent;

		int i = xfunc_XEventsQueued(x11MainDisplay, QueuedAfterFlush);
		if (i == 0) {
			xfunc_XUnlockDisplay(x11MainDisplay);
			int pollres = poll(polldata, 2, -1);
			ASSERT(pollres >= 0) << "poll call failed: " << strerror(errno);
			if (pollres < 0) {
				goto after_loop_unlocked;
			}

			if (HAS_FLAG(polldata[1].revents, POLLIN)) {
				//consume the event
				uint64 num;
				read(notifyeventfd, &num, 8);
			}
			xfunc_XLockDisplay(x11MainDisplay);
			i = xfunc_XEventsQueued(x11MainDisplay, QueuedAfterReading);
		}

		while (i-- > 0) {
			xfunc_XNextEvent(x11MainDisplay, &xevent);

			if (xfunc_XFilterEvent(&xevent, None)) {
				LOGTRACE()<< "Event filtered " << xevent.type;
				continue;
			}
			examine_event_skip_filter:

			switch (xevent.type) {
				case ConfigureNotify: {
					LOGI()<< "ConfigureNotify";
					auto& realevent = reinterpret_cast<XConfigureEvent&>(xevent);
					//xfunc_XUnlockDisplay(x11MainDisplay);
					mainThreadState.writeAppData<false>(
							CommandMessage<WindowStateCommandMessage> {APP_CMD_WINDOW_STATE_CHANGED, realevent.window, -1,
								(unsigned int) realevent.width, (unsigned int) realevent.height});
					//xfunc_XLockDisplay(x11MainDisplay);
					break;
				}
				case VisibilityNotify: {
					LOGI() << "VisibilityNotify";
					auto& realevent = reinterpret_cast<XVisibilityEvent&>(xevent);
					//xfunc_XUnlockDisplay(x11MainDisplay);
					mainThreadState.writeAppData<false>(
							CommandMessage<WindowStateCommandMessage> {APP_CMD_WINDOW_STATE_CHANGED, realevent.window,
								realevent.state == VisibilityFullyObscured ? 0 : 1, 0, 0});
					//xfunc_XLockDisplay(x11MainDisplay);
					break;
				}
				case KeyPress: {
					LOGI()<< "KeyPress";
					auto& realevent = reinterpret_cast<XKeyPressedEvent&>(xevent);
					KeySym sym;
					char buffer[16];
					int charcount = xfunc_XLookupString(&realevent, buffer, sizeof(buffer), &sym, &compose);
					postKeyEvent<KeyAction::DOWN>(realevent.window, sym);
					postUnicodeEvent(realevent.window, sym);
					break;
				}
				case KeyRelease: {
					LOGI() << "KeyRelease";
					auto& realevent = reinterpret_cast<XKeyReleasedEvent&>(xevent);
					KeySym sym;
					char buffer[16];
					int charcount = xfunc_XLookupString(&realevent, buffer, sizeof(buffer), &sym, &compose);
					//Key repeating posts an UP-DOWN with matching time field (millis)
					//if we are repeating, do not post the UP event
					if (i > 0) {
						--i;
						XEvent next;
						xfunc_XNextEvent(x11MainDisplay, &next);
						if (xfunc_XFilterEvent(&next, None)) {
							LOGTRACE()<< "Next event filtered " << next.type;
							postKeyEvent<KeyAction::UP>(realevent.window, sym);
						} else if (next.type == KeyPress && reinterpret_cast<XKeyPressedEvent&>(next).window == realevent.window
								&& reinterpret_cast<XKeyPressedEvent&>(next).time == realevent.time
								&& reinterpret_cast<XKeyPressedEvent&>(next).keycode == realevent.keycode
								&& reinterpret_cast<XKeyPressedEvent&>(next).state == realevent.state) {
							LOGTRACE()<< "Dropping KeyRelease for repeating character. keycode: "<< realevent.keycode;
							//the next event is going to be the corresponding DOWN
							//process the next DOWN event as usual

							//old goto method
							//xevent = next;
							//goto examine_event_skip_filter;

							auto& realevent = reinterpret_cast<XKeyPressedEvent&>(next);
							KeySym sym;
							char buffer[16];
							int charcount = xfunc_XLookupString(&realevent, buffer, sizeof(buffer), &sym, &compose);
							postKeyEvent<KeyAction::DOWN, true>(realevent.window, sym);
							postUnicodeEvent(realevent.window, sym);
							break;
						} else {
							postKeyEvent<KeyAction::UP>(realevent.window, sym);
							xevent = next;
							goto examine_event_skip_filter;
						}
					} else {
						postKeyEvent<KeyAction::UP>(realevent.window, sym);
					}
					break;
				}
				case ButtonPress: {
					auto& realevent = reinterpret_cast<XButtonPressedEvent&>(xevent);
					switch (realevent.button) {
						case Button1: {
							mainThreadState.writeAppData<false>(CommandMessage<MotionCommandMessage> {APP_CMD_MOTIONEVENT, realevent.window,
										InputDevice::MOUSE, 0, TouchAction::DOWN, (float) realevent.x, (float) realevent.y, getMonotonicTime()});
							break;
						}
						case Button4: {
							TouchEvent::ExtraData extra;
							extra.wheel = 1;
							extra.wheelMax = 1;
							mainThreadState.writeAppData<false>(CommandMessage<MotionExtraCommandMessage> {APP_CMD_MOTIONEVENT_EXTRA, realevent.window,
										InputDevice::MOUSE, 0, TouchAction::WHEEL, (float) realevent.x, (float) realevent.y, getMonotonicTime(), extra});
							break;
						}
						case Button5: {
							TouchEvent::ExtraData extra;
							extra.wheel = -1;
							extra.wheelMax = 1;
							mainThreadState.writeAppData<false>(CommandMessage<MotionExtraCommandMessage> {APP_CMD_MOTIONEVENT_EXTRA, realevent.window,
										InputDevice::MOUSE, 0, TouchAction::WHEEL, (float) realevent.x, (float) realevent.y, getMonotonicTime(), extra});
							break;
						}
						default: {
							break;
						}
					}
					if(realevent.button == Button1) {

					}
					break;
				}
				case ButtonRelease: {
					auto& realevent = reinterpret_cast<XButtonReleasedEvent&>(xevent);
					if(realevent.button == Button1) {
						mainThreadState.writeAppData<false>(CommandMessage<MotionCommandMessage> {APP_CMD_MOTIONEVENT, realevent.window,
									InputDevice::MOUSE, 0, TouchAction::UP, (float) realevent.x, (float) realevent.y, getMonotonicTime()});
					}
					break;
				}
				case MotionNotify: {
					auto time = getMonotonicTime();
					auto& realevent = reinterpret_cast<XPointerMovedEvent&>(xevent);
					if(HAS_FLAG(realevent.state, Button1Mask)) {
						mainThreadState.writeAppData<false>(CommandMessage<MotionCommandMessage> {APP_CMD_MOTIONEVENT, realevent.window,
									InputDevice::MOUSE, 0, TouchAction::MOVE_UPDATE, (float) realevent.x, (float) realevent.y, time});
						mainThreadState.writeAppData<false>(CommandMessage<MotionCommandMessage> {APP_CMD_MOTIONEVENT, realevent.window,
									InputDevice::MOUSE, -1, TouchAction::MOVE_UPDATE_DONE, 0.0f, 0.0f, time});
					} else {
						mainThreadState.writeAppData<false>(CommandMessage<MotionCommandMessage> {APP_CMD_MOTIONEVENT, realevent.window,
									InputDevice::MOUSE, 0, TouchAction::HOVER_MOVE, (float) realevent.x, (float) realevent.y, time});
					}
					break;
				}
				case DestroyNotify: {
					LOGI() << "DestroyNotify";
					bool exitapp;
					xfunc_XUnlockDisplay(x11MainDisplay);
					mainThreadState.writeAppData<true>(CommandMessage<WindowDestroyCommandMessage> {APP_CMD_WINDOW_DESTROYED,
								reinterpret_cast<XDestroyWindowEvent&>(xevent).window, &exitapp});
					xfunc_XLockDisplay(x11MainDisplay);

					if (exitapp) {
						goto after_loop;
					}

					break;
				}
				case ClientMessage: {
					LOGI() << "ClientMessage";
					if (xevent.xclient.data.l[0] == x11wmDeleteMessage) {
						bool exitapp;
						xfunc_XUnlockDisplay(x11MainDisplay);
						mainThreadState.writeAppData<true>(
								CommandMessage<WindowDestroyCommandMessage> {APP_CMD_WINDOW_DESTROYED, xevent.xclient.window, &exitapp});
						xfunc_XLockDisplay(x11MainDisplay);

						xfunc_XDestroyWindow(x11MainDisplay, xevent.xclient.window);
						XDEBUGSYNC(x11MainDisplay);
						if (exitapp) {
							goto after_loop;
						}
					}
					break;
				}
				default: {
					break;
				}
			}
		}
	}
	after_loop:

	xfunc_XUnlockDisplay(x11MainDisplay);
	after_loop_unlocked:

	LOGI()<< "Exited X11 event loop";

	//display is unlocked here
	mainThreadState.writeAppData<true>(EmptyCommandMessage { APP_CMD_TERMINATE });

	LOGI()<< "Closing X11 display";

	xfunc_XCloseDisplay(x11MainDisplay);

	dlclose(libOpenGLHandle);
	dlclose(libX11Handle);

	close(notifyeventfd);
	notifyX11FileDescriptor = -1;

	LOGI()<< "Exiting from main";
	LOG_MEMORY_LEAKS();
	return 0;
}
