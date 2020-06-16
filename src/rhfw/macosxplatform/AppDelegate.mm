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
//
//  AppDelegate.m
//
//  Created by Sipka on 2016. 08. 30..
//  Copyright © 2016. User. All rights reserved.
//

#include <macosxplatform/MacOsxNativeWindow.h>

#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/core/AppInterface.h>
#include <framework/core/timing.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/types.h>

#include <poll.h>
#include <errno.h>
#include <signal.h>

#include <mach/clock.h>
#include <mach/mach.h>

#import <AppKit/NSScreen.h>
#import <AppKit/NSTextInputContext.h>

class ProgramArguments {
public:
	int argc;
	char** argv;
};
static ProgramArguments ProgArgs;

namespace rhfw{
class platform_bridge{
	static Size2F dpi;
public:
	static void application_main(void* arg);
	
	static bool executeDrawing();
	
	static bool processInputData();

	static void initDpi();
};
Size2F platform_bridge::dpi {0, 0};
} // namespace rhfw

class MainThreadState{
	int msgpipe[2] { 0, 0 };
public:
	rhfw::Semaphore commandCompletedSemaphore;
	
	MainThreadState(){
		commandCompletedSemaphore.init();
		
		int res;
		
		res = pipe(msgpipe);
		ASSERT(res == 0) << "Failed to create pipe: " << strerror(errno);
		
		int flags = fcntl(getPipeReadFd(), F_GETFL, 0);
		res = fcntl(getPipeReadFd(), F_SETFL, flags | O_NONBLOCK);
		ASSERT(res == 0) << "Failed to set nonblocking pipe: " << strerror(errno);
	}
	~MainThreadState(){
		int res;
		res = close(msgpipe[0]);
		ASSERT(res == 0) << "Failed to close pipe: " << strerror(errno);
		res = close(msgpipe[1]);
		ASSERT(res == 0) << "Failed to close pipe: " << strerror(errno);
	}
	
	int getPipeReadFd() const{
		return msgpipe[0];
	}
	int getPipeWriteFd() const{
		return msgpipe[1];
	}
	
	template<bool wait, typename T>
	void writeAppData(const T& data) {
		if (write(this->getPipeWriteFd(), &data, sizeof(T)) != sizeof(T)) {
			LOGE() <<"Failure writing app_cmd: " << strerror(errno);
			THROW() <<"Failed to write app data";
		}
		if(wait){
			this->commandCompletedSemaphore.wait();
		}
	}
};
static MainThreadState mainThreadState;

static clock_serv_t clockService;

static rhfw::core::time_micros getMonotonicTime() {
	mach_timespec_t mts;
	clock_get_time(clockService, &mts);
	return rhfw::core::time_micros { (long long)mts.tv_sec * 1000000 + (long long)mts.tv_nsec / 1000};
}

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
	MacOsxNativeWindow* __unsafe_unretained window;
};
class __attribute((aligned(sizeof(void*)))) ContentSizeCommandMessage {
public:
	MacOsxNativeWindow* __unsafe_unretained window;
	unsigned int width;
	unsigned int height;
};
class __attribute((aligned(sizeof(void*)))) WindowVisibilityCommandMessage {
public:
	MacOsxNativeWindow* __unsafe_unretained window;
	bool visible;
};
class __attribute((aligned(sizeof(void*)))) TouchCommandMessage {
public:
	MacOsxNativeWindow* __unsafe_unretained window;
	rhfw::InputDevice device;
	int touchid;
	rhfw::TouchAction action;
	float x;
	float y;
	rhfw::core::time_micros millis;
};
class __attribute((aligned(sizeof(void*)))) TouchCommandExtraMessage {
public:
	MacOsxNativeWindow* __unsafe_unretained window;
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
	MacOsxNativeWindow* __unsafe_unretained window;
	rhfw::InputDevice device;
	rhfw::KeyAction action;
	rhfw::KeyMessage::ExtraData extra;
	rhfw::core::time_micros millis;
};
class __attribute((aligned(sizeof(void*)))) UnicodeCharMessage {
public:
	rhfw::core::Window* window;
	rhfw::UnicodeCodePoint codepoint;
};
class __attribute((aligned(sizeof(void*)))) UnicodeSequenceMessage {
public:
	rhfw::core::Window* window;
	rhfw::UnicodeCodePoint* codepoints;
	unsigned int length;
};
class __attribute((aligned(sizeof(void*)))) FullScreenChangedMessage {
public:
	rhfw::core::Window* window;
	bool fullscreen;
};

enum {
	APP_CMD_WINDOW_CREATED,
	APP_CMD_WINDOW_DESTROY,
	APP_CMD_WINDOW_CONTENTSIZE_CHANGED,
	APP_CMD_WINDOW_VISIBILITY_CHANGED,
	
	APP_CMD_WINDOW_FULLSCREEN_CHANGED,
	
	APP_CMD_TERMINATE,
	
	APP_CMD_TOUCH_ACTION,
	APP_CMD_KEY_ACTION,
	APP_CMD_TOUCH_ACTION_EXTRA,
	
	APP_CMD_UNICODE_CHAR,
	APP_CMD_UNICODE_SEQUENCE,
};

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end


@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	rhfw::Thread main_thread { rhfw::platform_bridge::application_main, nullptr };
	
	MacOsxNativeWindow* window = [[MacOsxNativeWindow alloc] initWithContentRect:[NSScreen mainScreen].frame];
	//[window toggleFullScreen:window];
	[window makeKeyAndOrderFront:nil];
	mainThreadState.writeAppData<true>(CommandMessage<WindowCommandMessage>{ APP_CMD_WINDOW_CREATED, window });
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
	mainThreadState.writeAppData<true>(EmptyCommandMessage{ APP_CMD_TERMINATE });
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication{
    return YES;
}
@end

@interface MacOsxNsApplication : NSApplication
@end

@implementation MacOsxNsApplication

- (void)sendEvent:(NSEvent *)event {
    if ([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask)){
        [[self keyWindow] sendEvent:event];
    }
	[super sendEvent:event];
}

@end

namespace rhfw {

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
bool platform_bridge::processInputData(){
	int fd = mainThreadState.getPipeReadFd();
	bool result = true;
	EmptyCommandMessage cmdmessage;
	size_t readres;
	while (result && (readres = read(fd, &cmdmessage, sizeof(cmdmessage))) == sizeof(cmdmessage)) {
		switch (cmdmessage.cmd) {
			case APP_CMD_WINDOW_CREATED: {
				LOGI() << "APP_CMD_WINDOW_CREATED"; 
				WindowCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow == nullptr){
						core::Window* w = new core::Window();
						msg.window.appwindow = w;
					
						auto time = getMonotonicTime();
						w->setNativeWindow(msg.window.nativewindow_cpp, time);
						
						core::Window::addWindow(*w, nullptr);
						
						w->setSize(core::WindowSize{Size2UI {msg.window.view.frame.size.width, msg.window.view.frame.size.height}, dpi});
						w->setVisibleToUser(true, time);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_CONTENTSIZE_CHANGED: {
				LOGI() << "APP_CMD_WINDOW_CONTENTSIZE_CHANGED";
				ContentSizeCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow == nullptr){
						core::Window* w = new core::Window();
						msg.window.appwindow = w;
						
						auto time = getMonotonicTime();
						w->setNativeWindow(msg.window.nativewindow_cpp, time);
						core::Window::addWindow(*w, nullptr);
						w->setVisibleToUser(true, time);
					}
					msg.window.appwindow->setSize(core::WindowSize{Size2UI {msg.width, msg.height}, dpi});
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_VISIBILITY_CHANGED: {
				LOGI() << "APP_CMD_WINDOW_VISIBILITY_CHANGED";
				WindowVisibilityCommandMessage msg;
				
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow != nullptr){
						msg.window.appwindow->setVisibleToUser(msg.visible, getMonotonicTime());
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_FULLSCREEN_CHANGED: {
				LOGI() << "APP_CMD_WINDOW_FULLSCREEN_CHANGED";
				FullScreenChangedMessage msg;
				
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if (msg.fullscreen){
						SET_FLAG(msg.window->windowStyle, WindowStyle::FULLSCREEN);
					}else{
						CLEAR_FLAG(msg.window->windowStyle, WindowStyle::FULLSCREEN);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_DESTROY: {
				LOGI() << "APP_CMD_WINDOW_DESTROY";
				WindowCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow != nullptr){
						core::Window::removeWindow(*msg.window.appwindow);
						//msg.window.nativewindow_cpp->window = nil;
						delete msg.window.appwindow;
						msg.window.appwindow = nullptr;
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_TERMINATE: {
				LOGI() << "APP_CMD_TERMINATE";
				user_app_terminate();
				result = false;
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_TOUCH_ACTION:{
				LOGI() << "APP_CMD_TOUCH_ACTION";
				TouchCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow != nullptr){
						rhfw::TouchMessage::postMessage(msg.window.appwindow, msg.device, msg.touchid, msg.action, msg.x, msg.y, msg.millis);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_TOUCH_ACTION_EXTRA:{
				LOGI() << "APP_CMD_TOUCH_ACTION_EXTRA";
				TouchCommandExtraMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow != nullptr){
						rhfw::TouchMessage::postMessage(msg.window.appwindow, msg.device, msg.touchid, msg.action, msg.x, msg.y, msg.millis, msg.extra);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_KEY_ACTION: {
				LOGI() << "APP_CMD_KEY_ACTION";
				KeyCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					//TODO do not ignore millis here
					if(msg.window.appwindow != nullptr){
						bool res = rhfw::KeyMessage::postMessage(msg.window.appwindow, msg.device, msg.action, msg.extra);
						if(!res && HAS_FLAG(msg.extra.modifiers, KeyModifiers::CTRL) && msg.extra.keycode == KeyCode::KEY_Q){
							//quit the app if CMD+Q is pressed and not handled
							//TODO do this differently with code interaction
							for(auto&& w : core::Window::getWindows().objects()){
								core::Window::removeWindow(w);
								w.getNativeWindow()->window.delegate = nil;
								w.getNativeWindow()->window.appwindow = nullptr;
								delete &w;
							}
							
							[[NSApplication sharedApplication] terminate:nil];
							//will receive APP_CMD_TERMINATE
						}
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_UNICODE_CHAR: {
				LOGI() << "APP_CMD_UNICODE_CHAR";
				UnicodeCharMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					KeyMessage::ExtraData extra;
					extra.repeatUnicode = msg.codepoint;
					extra.repeatUnicodeCount = 1;
					KeyMessage::postMessage(msg.window, InputDevice::KEYBOARD, KeyAction::UNICODE_REPEAT, extra);
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_UNICODE_SEQUENCE: {
				LOGI() << "APP_CMD_UNICODE_SEQUENCE";
				UnicodeSequenceMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					KeyMessage::ExtraData extra;
					extra.sequenceUnicode = msg.codepoints;
					extra.sequenceUnicodeCount = msg.length;
					KeyMessage::postMessage(msg.window, InputDevice::KEYBOARD, KeyAction::UNICODE_SEQUENCE, extra);
					delete[] msg.codepoints;
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			default:
				THROW() << "Invalid command message: " << (unsigned int)cmdmessage.cmd << " - " << cmdmessage.cmd;
				break;
		}
	}
	ASSERT(readres == sizeof(cmdmessage) || errno == EAGAIN)  << "No data on command pipe!, read: " << readres << " errno: " << strerror(errno);
	return result;
}
static FilePath MacOsxAppDataRoot;
const FilePath& AppleStorageDirectoryDescriptor::ApplicationDataRoot() {
	return MacOsxAppDataRoot;
}
FilePath AppleStorageDirectoryDescriptor::ApplicationDataDirectory(const char* appname) {
	return MacOsxAppDataRoot + appname;
}
void platform_bridge::application_main(void* arg){
	initDpi();
	MacOsxAppDataRoot = FilePath { [NSHomeDirectory() UTF8String] } + "Library";
	StorageDirectoryDescriptor::initializeRootDirectory(AppleStorageDirectoryDescriptor::ApplicationDataDirectory(APPLICATION_DEFAULT_NAME));
	StorageDirectoryDescriptor { StorageDirectoryDescriptor::Root() }.create();
	Thread::initApplicationMainThread();
	
	user_app_initialize(ProgArgs.argc, ProgArgs.argv);
	
	int res;
	struct pollfd polldata;
	polldata.fd = mainThreadState.getPipeReadFd();
	polldata.events = POLLIN;
	
	while(true) {
		core::GlobalMonotonicTimeListener::setCurrent(getMonotonicTime());
		const bool didDraw = executeDrawing();
		
		const bool waitInfinite = !core::GlobalMonotonicTimeListener::hasListeners() && !didDraw;
		
		res = poll(&polldata, 1, waitInfinite ? -1 : 0);
		ASSERT(res >= 0) << "poll call failed: " << strerror(errno);
		
		if(res > 0){
			//process data
			if(!processInputData()){
				break;
			}
		}
	}
	
}

void platform_bridge::initDpi(){
	NSScreen* screen = [NSScreen mainScreen];
	NSDictionary* description = [screen deviceDescription];
	NSSize displayPixelSize = [[description objectForKey:NSDeviceSize] sizeValue];
	CGSize displayPhysicalSize = CGDisplayScreenSize([[description objectForKey:@"NSScreenNumber"] unsignedIntValue]);
	
	dpi = Size2F { (displayPixelSize.width / displayPhysicalSize.width) * 25.4f, (displayPixelSize.height / displayPhysicalSize.height) * 25.4f };

	LOGI() << "DPI is: " << dpi;	
}

} // namespace rhfw

int main(int argc, char * argv[]) {
	ProgArgs.argc = argc;
	ProgArgs.argv = argv;
	signal(SIGPIPE, SIG_IGN);
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clockService);

	AppDelegate* delegate = [[AppDelegate alloc] init];
	[[MacOsxNsApplication sharedApplication] setDelegate:delegate];
	
	[[MacOsxNsApplication sharedApplication] run];
	
	mach_port_deallocate(mach_task_self(), clockService);
	
	return 0;
}

using namespace rhfw;


@implementation MacOsxNativeWindow{
	macosx_native_window_cpp nativewindow_cpp_impl;
	NSTextInputContext* inputcontext;
}

- (instancetype)initWithContentRect:(NSRect)contentRect {
	if([super initWithContentRect:contentRect styleMask:DEFAULT_WINDOW_STYLE backing:NSBackingStoreBuffered defer:NO]){
		self.appwindow = nullptr;		
		self.delegate = self;
		self.view = [[NSView alloc] initWithFrame:self.frame];
		nativewindow_cpp_impl.window = self;
		self.nativewindow_cpp = &nativewindow_cpp_impl;
		self.fullscreenOptions = NSApplicationPresentationFullScreen | NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock;
		[self setCollectionBehavior:(NSWindowCollectionBehaviorFullScreenPrimary)];
		[self setContentView:self.view];
		[self setAcceptsMouseMovedEvents:YES];
		[self setOpaque:YES];
		inputcontext = [[NSTextInputContext alloc] initWithClient:self];
		return self;
	}
	return nil;
}
- (void)dealloc{
	
	//no super dealloc with ARC
	//[super dealloc];
}

static const unichar KEYMAP_START = 0xF700;
static const unichar KEYMAP_END = 0xF748;
static KeyCode FUNCTION_KEY_MAP[] = {
/*NSUpArrowFunctionKey = 0xF700 */		KeyCode::KEY_DIR_UP,
/*NSDownArrowFunctionKey = 0xF701 */	KeyCode::KEY_DIR_DOWN,
/*NSLeftArrowFunctionKey = 0xF702 */	KeyCode::KEY_DIR_LEFT,
/*NSRightArrowFunctionKey = 0xF703 */	KeyCode::KEY_DIR_RIGHT,
/*NSF1FunctionKey = 0xF704 */			KeyCode::KEY_F1,
/*NSF2FunctionKey = 0xF705 */			KeyCode::KEY_F2,
/*NSF3FunctionKey = 0xF706 */			KeyCode::KEY_F3,
/*NSF4FunctionKey = 0xF707 */			KeyCode::KEY_F4,
/*NSF5FunctionKey = 0xF708 */			KeyCode::KEY_F5,
/*NSF6FunctionKey = 0xF709 */			KeyCode::KEY_F6,
/*NSF7FunctionKey = 0xF70A */			KeyCode::KEY_F7,
/*NSF8FunctionKey = 0xF70B */			KeyCode::KEY_F8,
/*NSF9FunctionKey = 0xF70C */			KeyCode::KEY_F9,
/*NSF10FunctionKey = 0xF70D */			KeyCode::KEY_F10,
/*NSF11FunctionKey = 0xF70E */			KeyCode::KEY_F11,
/*NSF12FunctionKey = 0xF70F */			KeyCode::KEY_F12,
/*NSF13FunctionKey = 0xF710 */			KeyCode::KEY_UNKNOWN,
/*NSF14FunctionKey = 0xF711 */			KeyCode::KEY_UNKNOWN,
/*NSF15FunctionKey = 0xF712 */			KeyCode::KEY_UNKNOWN,
/*NSF16FunctionKey = 0xF713 */			KeyCode::KEY_UNKNOWN,
/*NSF17FunctionKey = 0xF714 */			KeyCode::KEY_UNKNOWN,
/*NSF18FunctionKey = 0xF715 */			KeyCode::KEY_UNKNOWN,
/*NSF19FunctionKey = 0xF716 */			KeyCode::KEY_UNKNOWN,
/*NSF20FunctionKey = 0xF717 */			KeyCode::KEY_UNKNOWN,
/*NSF21FunctionKey = 0xF718 */			KeyCode::KEY_UNKNOWN,
/*NSF22FunctionKey = 0xF719 */			KeyCode::KEY_UNKNOWN,
/*NSF23FunctionKey = 0xF71A */			KeyCode::KEY_UNKNOWN,
/*NSF24FunctionKey = 0xF71B */			KeyCode::KEY_UNKNOWN,
/*NSF25FunctionKey = 0xF71C */			KeyCode::KEY_UNKNOWN,
/*NSF26FunctionKey = 0xF71D */			KeyCode::KEY_UNKNOWN,
/*NSF27FunctionKey = 0xF71E */			KeyCode::KEY_UNKNOWN,
/*NSF28FunctionKey = 0xF71F */			KeyCode::KEY_UNKNOWN,
/*NSF29FunctionKey = 0xF720 */			KeyCode::KEY_UNKNOWN,
/*NSF30FunctionKey = 0xF721 */			KeyCode::KEY_UNKNOWN,
/*NSF31FunctionKey = 0xF722 */			KeyCode::KEY_UNKNOWN,
/*NSF32FunctionKey = 0xF723 */			KeyCode::KEY_UNKNOWN,
/*NSF33FunctionKey = 0xF724 */			KeyCode::KEY_UNKNOWN,
/*NSF34FunctionKey = 0xF725 */			KeyCode::KEY_UNKNOWN,
/*NSF35FunctionKey = 0xF726 */			KeyCode::KEY_UNKNOWN,
/*NSInsertFunctionKey = 0xF727 */		KeyCode::KEY_UNKNOWN,
/*NSDeleteFunctionKey = 0xF728 */		KeyCode::KEY_DELETE,
/*NSHomeFunctionKey = 0xF729 */			KeyCode::KEY_HOME,
/*NSBeginFunctionKey = 0xF72A */		KeyCode::KEY_UNKNOWN,
/*NSEndFunctionKey = 0xF72B */			KeyCode::KEY_END,
/*NSPageUpFunctionKey = 0xF72C */		KeyCode::KEY_PAGE_UP,
/*NSPageDownFunctionKey = 0xF72D */		KeyCode::KEY_PAGE_DOWN,
/*NSPrintScreenFunctionKey = 0xF72E */	KeyCode::KEY_UNKNOWN,
/*NSScrollLockFunctionKey = 0xF72F */	KeyCode::KEY_SCROLLLOCK,
/*NSPauseFunctionKey = 0xF730 */		KeyCode::KEY_UNKNOWN,
/*NSSysReqFunctionKey = 0xF731 */		KeyCode::KEY_UNKNOWN,
/*NSBreakFunctionKey = 0xF732 */		KeyCode::KEY_UNKNOWN,
/*NSResetFunctionKey = 0xF733 */		KeyCode::KEY_UNKNOWN,
/*NSStopFunctionKey = 0xF734 */			KeyCode::KEY_UNKNOWN,
/*NSMenuFunctionKey = 0xF735 */			KeyCode::KEY_UNKNOWN,
/*NSUserFunctionKey = 0xF736 */			KeyCode::KEY_UNKNOWN,
/*NSSystemFunctionKey = 0xF737 */		KeyCode::KEY_UNKNOWN,
/*NSPrintFunctionKey = 0xF738 */		KeyCode::KEY_UNKNOWN,
/*NSClearLineFunctionKey = 0xF739 */	KeyCode::KEY_UNKNOWN,
/*NSClearDisplayFunctionKey = 0xF73A */	KeyCode::KEY_UNKNOWN,
/*NSInsertLineFunctionKey = 0xF73B */	KeyCode::KEY_UNKNOWN,
/*NSDeleteLineFunctionKey = 0xF73C */	KeyCode::KEY_UNKNOWN,
/*NSInsertCharFunctionKey = 0xF73D */	KeyCode::KEY_UNKNOWN,
/*NSDeleteCharFunctionKey = 0xF73E */	KeyCode::KEY_UNKNOWN,
/*NSPrevFunctionKey = 0xF73F */			KeyCode::KEY_UNKNOWN,
/*NSNextFunctionKey = 0xF740 */			KeyCode::KEY_UNKNOWN,
/*NSSelectFunctionKey = 0xF741 */		KeyCode::KEY_UNKNOWN,
/*NSExecuteFunctionKey = 0xF742 */		KeyCode::KEY_UNKNOWN,
/*NSUndoFunctionKey = 0xF743 */			KeyCode::KEY_UNKNOWN,
/*NSRedoFunctionKey = 0xF744 */			KeyCode::KEY_UNKNOWN,
/*NSFindFunctionKey = 0xF745 */			KeyCode::KEY_UNKNOWN,
/*NSHelpFunctionKey = 0xF746 */			KeyCode::KEY_UNKNOWN,
/*NSModeSwitchFunctionKey = 0xF747 */	KeyCode::KEY_UNKNOWN, 
};

static const KeyCode ASCII_KEYCODE_MAP[128] = {
/*	00	NUL	*/	KeyCode::KEY_UNKNOWN,				
/*	01	SOH	*/	KeyCode::KEY_UNKNOWN,				
/*	02	STX	*/	KeyCode::KEY_UNKNOWN,				
/*	03	ETX	*/	KeyCode::KEY_UNKNOWN,				
/*	04	EOT	*/	KeyCode::KEY_UNKNOWN,				
/*	05	ENQ	*/	KeyCode::KEY_UNKNOWN,				
/*	06	ACK	*/	KeyCode::KEY_UNKNOWN,				
/*	07	BEL	*/	KeyCode::KEY_UNKNOWN,				
/*	08	BS	*/	KeyCode::KEY_UNKNOWN,				
/*	09	HT	*/	KeyCode::KEY_TAB,				
/*	0A	LF	*/	KeyCode::KEY_UNKNOWN,				
/*	0B	VT	*/	KeyCode::KEY_UNKNOWN,				
/*	0C	FF	*/	KeyCode::KEY_UNKNOWN,				
/*	0D	CR	*/	KeyCode::KEY_ENTER,				
/*	0E	SO	*/	KeyCode::KEY_UNKNOWN,				
/*	0F	SI	*/	KeyCode::KEY_UNKNOWN,				
/*	10	DLE	*/	KeyCode::KEY_UNKNOWN,				
/*	11	DC1	*/	KeyCode::KEY_UNKNOWN,				
/*	12	DC2	*/	KeyCode::KEY_UNKNOWN,				
/*	13	DC3	*/	KeyCode::KEY_UNKNOWN,				
/*	14	DC4	*/	KeyCode::KEY_UNKNOWN,				
/*	15	NAK	*/	KeyCode::KEY_UNKNOWN,				
/*	16	SYN	*/	KeyCode::KEY_UNKNOWN,				
/*	17	ETB	*/	KeyCode::KEY_UNKNOWN,				
/*	18	CAN	*/	KeyCode::KEY_UNKNOWN,				
/*	19	EM	*/	KeyCode::KEY_UNKNOWN,				
/*	1A	SUB	*/	KeyCode::KEY_UNKNOWN,				
/*	1B	ESC	*/	KeyCode::KEY_ESC,				
/*	1C	FS	*/	KeyCode::KEY_UNKNOWN,				
/*	1D	GS	*/	KeyCode::KEY_UNKNOWN,				
/*	1E	RS	*/	KeyCode::KEY_UNKNOWN,				
/*	1F	US	*/	KeyCode::KEY_UNKNOWN,				
/*	20		*/	KeyCode::KEY_SPACE,				
/*	21	!	*/	KeyCode::KEY_UNKNOWN,				
/*	22	"	*/	KeyCode::KEY_UNKNOWN,				
/*	23	#	*/	KeyCode::KEY_UNKNOWN,				
/*	24	$	*/	KeyCode::KEY_UNKNOWN,				
/*	25	%	*/	KeyCode::KEY_UNKNOWN,				
/*	26	&	*/	KeyCode::KEY_UNKNOWN,				
/*	27	'	*/	KeyCode::KEY_UNKNOWN,				
/*	28	(	*/	KeyCode::KEY_UNKNOWN,				
/*	29	)	*/	KeyCode::KEY_UNKNOWN,				
/*	2A	*	*/	KeyCode::KEY_UNKNOWN,				
/*	2B	+	*/	KeyCode::KEY_UNKNOWN,				
/*	2C	,	*/	KeyCode::KEY_UNKNOWN,				
/*	2D	-	*/	KeyCode::KEY_UNKNOWN,				
/*	2E	.	*/	KeyCode::KEY_UNKNOWN,				
/*	2F	/	*/	KeyCode::KEY_UNKNOWN,				
/*	30	0	*/	KeyCode::KEY_0,				
/*	31	1	*/	KeyCode::KEY_1,				
/*	32	2	*/	KeyCode::KEY_2,				
/*	33	3	*/	KeyCode::KEY_3,				
/*	34	4	*/	KeyCode::KEY_4,				
/*	35	5	*/	KeyCode::KEY_5,				
/*	36	6	*/	KeyCode::KEY_6,				
/*	37	7	*/	KeyCode::KEY_7,				
/*	38	8	*/	KeyCode::KEY_8,				
/*	39	9	*/	KeyCode::KEY_9,				
/*	3A	:	*/	KeyCode::KEY_UNKNOWN,				
/*	3B	;	*/	KeyCode::KEY_UNKNOWN,				
/*	3C	<	*/	KeyCode::KEY_UNKNOWN,				
/*	3D	=	*/	KeyCode::KEY_UNKNOWN,				
/*	3E	>	*/	KeyCode::KEY_UNKNOWN,				
/*	3F	?	*/	KeyCode::KEY_UNKNOWN,				
/*	40	@	*/	KeyCode::KEY_UNKNOWN,				
/*	41	A	*/	KeyCode::KEY_A,				
/*	42	B	*/	KeyCode::KEY_B,				
/*	43	C	*/	KeyCode::KEY_C,				
/*	44	D	*/	KeyCode::KEY_D,				
/*	45	E	*/	KeyCode::KEY_E,				
/*	46	F	*/	KeyCode::KEY_F,				
/*	47	G	*/	KeyCode::KEY_G,				
/*	48	H	*/	KeyCode::KEY_H,				
/*	49	I	*/	KeyCode::KEY_I,				
/*	4A	J	*/	KeyCode::KEY_J,				
/*	4B	K	*/	KeyCode::KEY_K,				
/*	4C	L	*/	KeyCode::KEY_L,				
/*	4D	M	*/	KeyCode::KEY_M,				
/*	4E	N	*/	KeyCode::KEY_N,				
/*	4F	O	*/	KeyCode::KEY_O,				
/*	50	P	*/	KeyCode::KEY_P,				
/*	51	Q	*/	KeyCode::KEY_Q,				
/*	52	R	*/	KeyCode::KEY_R,				
/*	53	S	*/	KeyCode::KEY_S,				
/*	54	T	*/	KeyCode::KEY_T,				
/*	55	U	*/	KeyCode::KEY_U,				
/*	56	V	*/	KeyCode::KEY_V,				
/*	57	W	*/	KeyCode::KEY_W,				
/*	58	X	*/	KeyCode::KEY_X,				
/*	59	Y	*/	KeyCode::KEY_Y,				
/*	5A	Z	*/	KeyCode::KEY_Z,				
/*	5B	[	*/	KeyCode::KEY_UNKNOWN,				
/*	5C	\	*/	KeyCode::KEY_UNKNOWN,				
/*	5D	]	*/	KeyCode::KEY_UNKNOWN,				
/*	5E	^	*/	KeyCode::KEY_UNKNOWN,				
/*	5F	_	*/	KeyCode::KEY_UNKNOWN,				
/*	60	`	*/	KeyCode::KEY_UNKNOWN,				
/*	61	a	*/	KeyCode::KEY_A,				
/*	62	b	*/	KeyCode::KEY_B,				
/*	63	c	*/	KeyCode::KEY_C,				
/*	64	d	*/	KeyCode::KEY_D,				
/*	65	e	*/	KeyCode::KEY_E,				
/*	66	f	*/	KeyCode::KEY_F,				
/*	67	g	*/	KeyCode::KEY_G,				
/*	68	h	*/	KeyCode::KEY_H,				
/*	69	i	*/	KeyCode::KEY_I,				
/*	6A	j	*/	KeyCode::KEY_J,				
/*	6B	k	*/	KeyCode::KEY_K,				
/*	6C	l	*/	KeyCode::KEY_L,				
/*	6D	m	*/	KeyCode::KEY_M,				
/*	6E	n	*/	KeyCode::KEY_N,				
/*	6F	o	*/	KeyCode::KEY_O,				
/*	70	p	*/	KeyCode::KEY_P,				
/*	71	q	*/	KeyCode::KEY_Q,				
/*	72	r	*/	KeyCode::KEY_R,				
/*	73	s	*/	KeyCode::KEY_S,				
/*	74	t	*/	KeyCode::KEY_T,				
/*	75	u	*/	KeyCode::KEY_U,				
/*	76	v	*/	KeyCode::KEY_V,				
/*	77	w	*/	KeyCode::KEY_W,				
/*	78	x	*/	KeyCode::KEY_X,				
/*	79	y	*/	KeyCode::KEY_Y,				
/*	7A	z	*/	KeyCode::KEY_Z,				
/*	7B	{	*/	KeyCode::KEY_UNKNOWN,				
/*	7C	|	*/	KeyCode::KEY_UNKNOWN,				
/*	7D	}	*/	KeyCode::KEY_UNKNOWN,				
/*	7E	~	*/	KeyCode::KEY_UNKNOWN,				
/*	7F		*/	KeyCode::KEY_BACKSPACE,				
};

- (BOOL) handleKeyEvent:(NSEvent *)theEvent action:(KeyAction) keyAction {
	NSString* chars = [theEvent charactersIgnoringModifiers];
	if ([chars length] == 0)
		return NO;
	if ([chars length] == 1) {
		unichar keyChar = [chars characterAtIndex:0];
		auto millis = getMonotonicTime();
		KeyMessage::ExtraData extra;
		if(keyChar >= KEYMAP_START && keyChar < KEYMAP_END) {
			extra.keycode = FUNCTION_KEY_MAP[keyChar - KEYMAP_START];
		} else if(keyChar <= 0x7F) {
			extra.keycode = ASCII_KEYCODE_MAP[keyChar];
		} else {
			extra.keycode = KeyCode::KEY_UNKNOWN;
		}
		extra.repeat = [theEvent isARepeat];
		extra.modifiers = KeyModifiers::NO_FLAG;
		auto eventmod = theEvent.modifierFlags;
		if (HAS_FLAG(eventmod, NSAlphaShiftKeyMask)){
			extra.modifiers |= KeyModifiers::CAPSLOCK;
		}
		if (HAS_FLAG(eventmod, NSShiftKeyMask)) {
			extra.modifiers |= KeyModifiers::SHIFT;
		}
		if (HAS_FLAG(eventmod, NSCommandKeyMask)) {
			extra.modifiers |= KeyModifiers::CTRL;
		}
		if (HAS_FLAG(eventmod, NSAlternateKeyMask )) {
			extra.modifiers |= KeyModifiers::ALT;
		}
		mainThreadState.writeAppData<false>(CommandMessage<KeyCommandMessage>{ APP_CMD_KEY_ACTION, self, InputDevice::KEYBOARD, keyAction, extra, millis});
		return YES;
	}
	return NO;
}

- (void)keyDown:(NSEvent *)theEvent{
	if (![self handleKeyEvent:theEvent action:KeyAction::DOWN]) {
		[super keyDown:theEvent];
	}
	[inputcontext handleEvent:theEvent];
}
- (void)keyUp:(NSEvent *)theEvent{
	if (![self handleKeyEvent:theEvent action:KeyAction::UP]) {
		[super keyUp:theEvent];
	}
}

#define SEND_MODIFIER_KEYEVENT(mask, keycode_) \
if(HAS_FLAG(changes, mask)) {\
	KeyMessage::ExtraData extra;\
	extra.keycode = keycode_;\
	mainThreadState.writeAppData<false>(CommandMessage<KeyCommandMessage>{ APP_CMD_KEY_ACTION, self,\
											rhfw::InputDevice::KEYBOARD, HAS_FLAG(nflags, mask) ? KeyAction::DOWN : KeyAction::UP, extra, millis});\
}

/**
 * To monitor Command, Alt, Shift and other key changes
 */
static NSEventModifierFlags modifiers_flags = 0;
- (void)flagsChanged:(NSEvent *)theEvent{
	auto millis = getMonotonicTime();
	NSEventModifierFlags nflags = [theEvent modifierFlags];
	NSEventModifierFlags changes = nflags ^ modifiers_flags;
	
	SEND_MODIFIER_KEYEVENT(NSControlKeyMask, KeyCode::KEY_CTRL);
	SEND_MODIFIER_KEYEVENT(NSAlternateKeyMask, KeyCode::KEY_ALT);
	SEND_MODIFIER_KEYEVENT(NSCommandKeyMask, KeyCode::KEY_COMMAND);
	SEND_MODIFIER_KEYEVENT(NSShiftKeyMask, KeyCode::KEY_SHIFT);
	
	modifiers_flags = nflags;
}

- (void)mouseDown:(NSEvent *)theEvent{
	auto loc = [theEvent locationInWindow];
	auto millis = getMonotonicTime();
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::MOUSE, 0, rhfw::TouchAction::DOWN, 
											(float) loc.x, (float) (self.view.frame.size.height - loc.y), millis});
}
- (void)mouseUp:(NSEvent *)theEvent{
	auto loc = [theEvent locationInWindow];
	auto millis = getMonotonicTime();
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::MOUSE, 0, rhfw::TouchAction::UP, 
											(float) loc.x, (float) (self.view.frame.size.height - loc.y), millis});
}

- (void)mouseDragged:(NSEvent *)theEvent{
	auto loc = [theEvent locationInWindow];
	auto millis = getMonotonicTime();
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::MOUSE, 0, rhfw::TouchAction::MOVE_UPDATE, 
											(float) loc.x, (float) (self.view.frame.size.height - loc.y), millis});
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::MOUSE, -1, rhfw::TouchAction::MOVE_UPDATE_DONE, 
											0.0f, 0.0f, millis});
}
- (void)mouseMoved:(NSEvent *)theEvent{
	auto loc = [theEvent locationInWindow];
	auto millis = getMonotonicTime();
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::MOUSE, 0, rhfw::TouchAction::HOVER_MOVE, 
											(float) loc.x, (float) (self.view.frame.size.height - loc.y), millis});
}

- (void)rightMouseUp:(NSEvent *)theEvent {
}
- (void)otherMouseUp:(NSEvent *)theEvent {
}
- (void)scrollWheel:(NSEvent *)theEvent {
	auto loc = [theEvent locationInWindow];
	auto millis = getMonotonicTime();
	rhfw::TouchEvent::ExtraData extra;
	extra.scrollHorizontal = [theEvent deltaX] * 4.0f;
	extra.scrollVertical = [theEvent deltaY] * 4.0f;
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandExtraMessage>{ APP_CMD_TOUCH_ACTION_EXTRA, self, 
										rhfw::InputDevice::MOUSE, 0, rhfw::TouchAction::SCROLL, 
										(float) loc.x, (float) (self.view.frame.size.height - loc.y), millis, extra});
}

- (BOOL)canBecomeKeyWindow{
	return YES;
}
- (BOOL)canBecomeMainWindow{
	return YES;
}
- (BOOL)acceptsFirstResponder {
    return YES;
}
- (BOOL)becomeFirstResponder{
	return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent{
	return YES;
}

- (void)windowDidResize:(NSNotification *)notification {
	MacOsxNativeWindow* window = [notification object];
	mainThreadState.writeAppData<true>(CommandMessage<ContentSizeCommandMessage>{ APP_CMD_WINDOW_CONTENTSIZE_CHANGED, window, 
				(unsigned int)(window.view.frame.size.width + 0.5f), (unsigned int)(window.view.frame.size.height + 0.5f) });
}
- (NSApplicationPresentationOptions)window:(NSWindow *)window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions)proposedOptions{
	return self.fullscreenOptions;
}
- (void)windowDidEnterFullScreen:(NSNotification *)notification {
	mainThreadState.writeAppData<true>(CommandMessage<FullScreenChangedMessage>{ APP_CMD_WINDOW_FULLSCREEN_CHANGED, self.appwindow, true });
}
- (void)windowDidExitFullScreen:(NSNotification *)notification{
	mainThreadState.writeAppData<true>(CommandMessage<FullScreenChangedMessage>{ APP_CMD_WINDOW_FULLSCREEN_CHANGED, self.appwindow, false });
}

- (void)windowDidDeminiaturize:(NSNotification *)notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowVisibilityCommandMessage>{ APP_CMD_WINDOW_VISIBILITY_CHANGED, self, HAS_FLAG(self.occlusionState, NSWindowOcclusionStateVisible) });
}

- (void)windowDidMiniaturize:(NSNotification *)notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowVisibilityCommandMessage>{ APP_CMD_WINDOW_VISIBILITY_CHANGED, self, false });
}

- (void)windowDidChangeOcclusionState:(NSNotification *)notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowVisibilityCommandMessage>{ APP_CMD_WINDOW_VISIBILITY_CHANGED, self, HAS_FLAG(self.occlusionState, NSWindowOcclusionStateVisible) });
}

- (void)windowWillClose:(NSNotification *)notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowCommandMessage>{ APP_CMD_WINDOW_DESTROY, self });
	self.delegate = nil;
	//nativewindow_cpp->window = nil;
}

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff
#define IS_HIGH_SURROGATE(wch) (((wch) >= HIGH_SURROGATE_START) && ((wch) <= HIGH_SURROGATE_END))
#define IS_LOW_SURROGATE(wch)  (((wch) >= LOW_SURROGATE_START) && ((wch) <= LOW_SURROGATE_END))
#define IS_SURROGATE_PAIR(hs, ls) (IS_HIGH_SURROGATE(hs) && IS_LOW_SURROGATE(ls))
- (void)insertText:(id)text replacementRange:(NSRange)replacementRange{
	[self insertText:text];
}
- (void)insertText:(id)text{
	// never called?
	unsigned int length = [text length];
    if (length == 1){
    	unichar uc = [text characterAtIndex: 0];
		mainThreadState.writeAppData<false>(CommandMessage<UnicodeCharMessage>{APP_CMD_UNICODE_CHAR, self.appwindow, uc});
    } else {
    	unsigned int lastsurrogate = 0;
    	unichar buffer[length + 1];
    	rhfw::UnicodeCodePoint* codepoints = new rhfw::UnicodeCodePoint[length]; 
    	unsigned int offset = 0;

		[text getCharacters:buffer range:NSMakeRange(0, length)];
    	for (unsigned int i = 0; i < length; ++i) {
    		if (IS_HIGH_SURROGATE(buffer[i])) {
    			lastsurrogate = buffer[i];
    		} else {
    			if (IS_SURROGATE_PAIR(lastsurrogate, buffer[i])) {
    				codepoints[offset++] = 0x10000 | ((lastsurrogate & 0x3FF) << 10) | (buffer[i] & 0x3FF);
    				lastsurrogate = 0;
    			} else {
    				codepoints[offset++] = buffer[i];
    			}
    		}
    	}
    	mainThreadState.writeAppData<false>(CommandMessage<UnicodeSequenceMessage>{APP_CMD_UNICODE_SEQUENCE, self.appwindow, codepoints, offset});
    }
}
- (void)doCommandBySelector:(SEL)aSelector{
}
- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange{
}
- (void)unmarkText{
}
- (NSRange)selectedRange{
	return {NSNotFound, 0};
}
- (NSRange)markedRange{
	return {NSNotFound, 0};
}
- (BOOL)hasMarkedText{
	return NO;
}
- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(nullable NSRangePointer)actualRange{
	return nil;
}
- (NSArray<NSString *> *)validAttributesForMarkedText{
	return [NSArray<NSString*> new];
}
- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(nullable NSRangePointer)actualRange{
	return NSRect{0,0,0,0};
}
- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint{
	return 0;
}


@end
