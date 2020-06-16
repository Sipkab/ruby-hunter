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
//  TestApp
//
//  Created by User on 2016. 02. 17..
//  Copyright © 2016. User. All rights reserved.
//

#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/core/AppInterface.h>
#include <framework/core/timing.h>

#include <gen/log.h>
//#include <gen/resconfig.h>
//#include <gen/modules.h>

#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/utsname.h>

#include <framework/core/Window.h>

#import <iosplatform/AppDelegate.h>

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
	bool isAppForeground = true;
	
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
		if (wait) {
			this->commandCompletedSemaphore.wait();
		}
	}
};
static rhfw::core::time_micros getMonotonicTime() {
	return rhfw::core::time_micros{static_cast<long long>(CACurrentMediaTime() * 1000000)};
}

static MainThreadState mainThreadState;

//packing can cause crashes
//align against error EXC_ARM_DA_ALIGN
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
	MyNewWindow* __unsafe_unretained window;
};
class __attribute((aligned(sizeof(void*)))) WindowVisibilityCommandMessage {
public:
	MyNewWindow* __unsafe_unretained window;
	bool visible;
};
class __attribute((aligned(sizeof(void*)))) LayerCommandMessage {
public:
	MyNewEAGLLayer* __unsafe_unretained layer;
};
class __attribute((aligned(sizeof(void*)))) SizeCommandMessage {
public:
	MyNewEAGLLayer* __unsafe_unretained layer;
	unsigned int width;
	unsigned int height;
};
class __attribute((aligned(sizeof(void*)))) LayerCreatedCommandMessage {
public:
	MyNewEAGLLayer* __unsafe_unretained layer;
	unsigned int width;
	unsigned int height;
};
class __attribute((aligned(sizeof(void*)))) AppStateChangedCommandMessage {
public:
	bool foreground;
};

class __attribute((aligned(sizeof(void*)))) TouchCommandMessage {
public:
	MyNewWindow* __unsafe_unretained window;
	rhfw::InputDevice device;
	int touchid;
	rhfw::TouchAction action;
	float x;
	float y;
	rhfw::core::time_micros millis;
};

class __attribute((aligned(sizeof(void*)))) KeyUpDownMessage {
public:
	rhfw::core::Window* window;
	rhfw::KeyCode keycode;
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

enum {
	APP_CMD_WINDOW_CREATED,
	APP_CMD_WINDOW_DESTROYING,
	
	APP_CMD_LAYER_CREATED,
	APP_CMD_LAYER_DESTROYING,
	
	APP_CMD_WINDOW_VISIBILITY_CHANGED,
	APP_CMD_LAYER_SIZE_CHANGED,
	
	APP_CMD_TERMINATE,
	
	APP_CMD_TOUCH_ACTION,
	
	APP_CMD_STATE_CHANGED,
	
	APP_CMD_KEYUPDOWN,
	APP_CMD_UNICODE_CHAR,
	APP_CMD_UNICODE_SEQUENCE,
};


static CGFloat scale = [[UIScreen mainScreen] scale];
@implementation MyNewEAGLLayer {
	CGSize msize;
	bool inited;
	ios_native_layer nativelayervalue;
}

- (instancetype)init
{
	self = [super init];
	if (self) {
		inited = false;
		self.nativeLayer = &nativelayervalue;
		nativelayervalue = { self };
		super.opaque = true;
		super.contentsScale = scale;
	}
	return self;
}
- (void)dealloc{
	mainThreadState.writeAppData<true>(CommandMessage<LayerCommandMessage>{APP_CMD_LAYER_DESTROYING, self });
	//no super dealloc with ARC
	//[super dealloc];
}

- (void)setContentsScale:(CGFloat)contentsScale{
}
- (void)setOpaque:(BOOL)opaque{
}
- (void)setBounds:(CGRect)bounds{
	CGSize nsize = CGSizeMake(bounds.size.width * scale, bounds.size.height * scale);
	if(!CGSizeEqualToSize(msize, nsize)){
		super.bounds = bounds;
		msize = nsize;
		
		unsigned int uw = (unsigned int)(nsize.width + 0.5f);
		unsigned int uh = (unsigned int)(nsize.height + 0.5f);
		
		if(!inited){
			mainThreadState.writeAppData<true>(CommandMessage<LayerCreatedCommandMessage>{ APP_CMD_LAYER_CREATED, self, uw, uh });
			inited = true;
		}else{
			mainThreadState.writeAppData<true>(CommandMessage<SizeCommandMessage>{ APP_CMD_LAYER_SIZE_CHANGED, self, uw, uh });
		}
	}
}

@end




@implementation MyNewWindow{
	//make big enough array
	int ids[rhfw::TouchEvent::MAX_POINTER_COUNT];
	unsigned int idsReleased;
	UITouch* touches[rhfw::TouchEvent::MAX_POINTER_COUNT];
}

+ (Class)layerClass {
	return MyNewEAGLLayer.class;
}

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff
#define IS_HIGH_SURROGATE(wch) (((wch) >= HIGH_SURROGATE_START) && ((wch) <= HIGH_SURROGATE_END))
#define IS_LOW_SURROGATE(wch)  (((wch) >= LOW_SURROGATE_START) && ((wch) <= LOW_SURROGATE_END))
#define IS_SURROGATE_PAIR(hs, ls) (IS_HIGH_SURROGATE(hs) && IS_LOW_SURROGATE(ls))

- (void)insertText:(NSString *)text {
    // Do something with the typed character
    unsigned int length = [text length];
    if (length == 1){
    	unichar uc = [text characterAtIndex: 0];
    	if(uc == '\n'){
    		mainThreadState.writeAppData<false>(CommandMessage<KeyUpDownMessage>{APP_CMD_KEYUPDOWN, self.appwindow, rhfw::KeyCode::KEY_ENTER});
    	}else{
    		mainThreadState.writeAppData<false>(CommandMessage<UnicodeCharMessage>{APP_CMD_UNICODE_CHAR, self.appwindow, uc});
		}
    } else {
    	unsigned int lastsurrogate = 0;
    	unichar buffer[length + 1];
    	rhfw::UnicodeCodePoint* codepoints = new rhfw::UnicodeCodePoint[length]; 
    	unsigned int offset = 0;

		[text getCharacters:buffer range:NSMakeRange(0, length)];
    	for(unsigned int i = 0; i < length; ++i){
    		if(IS_HIGH_SURROGATE(buffer[i])){
    			lastsurrogate = buffer[i];
    		}else{
    			if (IS_SURROGATE_PAIR(lastsurrogate, buffer[i])) {
    				codepoints[offset++] = 0x10000 | ((lastsurrogate & 0x3FF) << 10) | (buffer[i] & 0x3FF);
    				lastsurrogate = 0;
    			}else{
    				codepoints[offset++] = buffer[i];
    			}
    		}
    	}
    	mainThreadState.writeAppData<false>(CommandMessage<UnicodeSequenceMessage>{APP_CMD_UNICODE_SEQUENCE, self.appwindow, codepoints, offset});
    }
}
- (void)deleteBackward {
	mainThreadState.writeAppData<false>(CommandMessage<KeyUpDownMessage>{APP_CMD_KEYUPDOWN, self.appwindow, rhfw::KeyCode::KEY_BACKSPACE});
}
- (BOOL)hasText {
    // Return whether there's any text present
    return YES;
}

- (instancetype)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];
	if (self) {
		idsReleased = 0;
		for (int i = 0; i < rhfw::TouchEvent::MAX_POINTER_COUNT; ++i) {
			ids[i] = i;
		}
		
		self.keyboardType = UIKeyboardTypeASCIICapable;
		
		self.multipleTouchEnabled = true;
		self.exclusiveTouch = true;
		
		self.rootViewController = [[UIViewController alloc] init];
		
		((MyNewEAGLLayer*)self.layer).nativeLayer->window = self;
		self.appwindow = ((MyNewEAGLLayer*)self.layer).appwindow;
		ASSERT(self.appwindow != nullptr) << "App window was not created";
		
		//comes with APP_CMD_LAYER_CREATED
		//mainThreadState.writeAppData<true>(CommandMessage<WindowCommandMessage>{ APP_CMD_WINDOW_CREATED, self });
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appWindowBecameVisible:) name:UIWindowDidBecomeVisibleNotification object:self];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appWindowBecameHidden:) name:UIWindowDidBecomeHiddenNotification object:self];
	}
	return self;
}
- (void)dealloc{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	mainThreadState.writeAppData<true>(CommandMessage<WindowCommandMessage>{ APP_CMD_WINDOW_DESTROYING, self });
	//no super dealloc with ARC
	//[super dealloc];
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void) appWindowBecameVisible:(NSNotification *) notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowVisibilityCommandMessage>{ APP_CMD_WINDOW_VISIBILITY_CHANGED, self, true });
}
- (void) appWindowBecameHidden:(NSNotification *) notification{
	mainThreadState.writeAppData<true>(CommandMessage<WindowVisibilityCommandMessage>{ APP_CMD_WINDOW_VISIBILITY_CHANGED, self, false });
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event{
	return self;
}
- (void)touchesBegan:(NSSet<UITouch *> *)touchesset withEvent:(UIEvent *)event{
	auto millis = getMonotonicTime();
	for (UITouch* touch in touchesset) {
		ASSERT(idsReleased < rhfw::TouchEvent::MAX_POINTER_COUNT) << "No id to release for touch down event";
		int touchid = ids[idsReleased];
		touches[touchid] = touch;
		++idsReleased;
		
		CGPoint pos = [touch locationInView:self];
		mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
											rhfw::InputDevice::FINGER, touchid, rhfw::TouchAction::DOWN, 
											static_cast<float>(pos.x * scale), static_cast<float>(pos.y * scale), millis});
	}
}
- (void)touchesMoved:(NSSet<UITouch *> *)touchesset withEvent:(UIEvent *)event{
	auto millis = getMonotonicTime();
	for (UITouch* touch in touchesset) {
		for (int i = 0; i < rhfw::TouchEvent::MAX_POINTER_COUNT; ++i) {
			UITouch* ltouch = touches[i];
			if(ltouch != touch)
				continue;
			//i == id
			
			CGPoint pos = [touch locationInView:self];
			mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
												rhfw::InputDevice::FINGER, i, rhfw::TouchAction::MOVE_UPDATE, 
												static_cast<float>(pos.x * scale), static_cast<float>(pos.y * scale), millis });
		}
	}
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
																			rhfw::InputDevice::FINGER, -1, rhfw::TouchAction::MOVE_UPDATE_DONE,
																			0.0f, 0.0f, millis });
}
- (void)touchesEnded:(NSSet<UITouch *> *)touchesset withEvent:(UIEvent *)event{
	auto millis = getMonotonicTime();
	for (UITouch* touch in touchesset) {
		for (int i = 0; i < rhfw::TouchEvent::MAX_POINTER_COUNT; ++i) {
			UITouch* ltouch = touches[i];
			if(ltouch != touch)
				continue;
			//i == id
			ASSERT(idsReleased>0) << "Trying to end unreleased touch event";
			--idsReleased;
			touches[i] = nullptr;
			ids[idsReleased] = i;
			

			CGPoint pos = [touch locationInView:self];
			mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
																					rhfw::InputDevice::FINGER, i, rhfw::TouchAction::UP, 
																					static_cast<float>(pos.x * scale), static_cast<float>(pos.y * scale), millis });
		}
	}
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touchesset withEvent:(UIEvent *)event{
	for (int i = 0; i < rhfw::TouchEvent::MAX_POINTER_COUNT; ++i) {
		ids[i] = (unsigned int)i;
		touches[i] = nullptr;
	}
	idsReleased = 0;
	mainThreadState.writeAppData<false>(CommandMessage<TouchCommandMessage>{ APP_CMD_TOUCH_ACTION, self, 
																			rhfw::InputDevice::FINGER, -1, rhfw::TouchAction::CANCEL, 
																			0.0f, 0.0f, getMonotonicTime() });
}

@end



@interface AppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation AppDelegate {

	//TODO ezt local-ba?
	MyNewWindow* window;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	rhfw::Thread main_thread { rhfw::platform_bridge::application_main, nullptr };
	window = [[MyNewWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	// Override point for customization after application launch.

	[window makeKeyAndVisible];
	[window resignFirstResponder];
	return YES;
}
- (void)applicationWillResignActive:(UIApplication *)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.

	mainThreadState.writeAppData<true>(CommandMessage<AppStateChangedCommandMessage>{APP_CMD_STATE_CHANGED, false });
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.

	mainThreadState.writeAppData<true>(CommandMessage<AppStateChangedCommandMessage>{APP_CMD_STATE_CHANGED, true });
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
	mainThreadState.writeAppData<true>(EmptyCommandMessage{ APP_CMD_TERMINATE });
}

@end

namespace rhfw{

bool platform_bridge::processInputData(){
	int fd = mainThreadState.getPipeReadFd();
	bool result = true;
	EmptyCommandMessage cmdmessage;
	size_t readres;
	while (result && (readres = read(fd, &cmdmessage, sizeof(cmdmessage))) == sizeof(cmdmessage)) {
		switch (cmdmessage.cmd) {
			case APP_CMD_WINDOW_CREATED:{
				WindowCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.window.appwindow == nullptr){
						core::Window* w = new core::Window();
						msg.window.appwindow = w;
					
						core::Window::addWindow(*w, nullptr);
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_DESTROYING:{
				WindowCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					core::Window::removeWindow(*msg.window.appwindow);
					delete msg.window.appwindow;
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_LAYER_CREATED: {
				LayerCreatedCommandMessage msg;

				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					if(msg.layer.appwindow == nullptr){
						core::Window* w = new core::Window();
						msg.layer.appwindow = w;
						
						core::Window::addWindow(*w, nullptr);
					}
					msg.layer.appwindow->setNativeLayer(msg.layer.nativeLayer, getMonotonicTime());
					msg.layer.appwindow->setSize(core::WindowSize{Size2UI {msg.width, msg.height}, dpi});
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_LAYER_DESTROYING: {
				LayerCommandMessage msg;
				
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					msg.layer.appwindow->setNativeLayer(nullptr, getMonotonicTime());
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_LAYER_SIZE_CHANGED: {
				SizeCommandMessage msg;
				
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					msg.layer.appwindow->setSize(core::WindowSize{Size2UI {msg.width, msg.height}, dpi});
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_WINDOW_VISIBILITY_CHANGED: {
				WindowVisibilityCommandMessage msg;
				
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					msg.window.appwindow->setVisibleToUser(mainThreadState.isAppForeground && msg.visible, getMonotonicTime());
					msg.window.appwindow->getNativeLayer()->isVisible = msg.visible;
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_TERMINATE:{
				user_app_terminate();
				result = false;
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_TOUCH_ACTION:{
				TouchCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					rhfw::TouchMessage::postMessage(msg.window.appwindow, msg.device, msg.touchid, msg.action, msg.x, msg.y, msg.millis);
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_STATE_CHANGED:{
				AppStateChangedCommandMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					mainThreadState.isAppForeground = msg.foreground;
					for(auto& w : rhfw::core::Window::getWindows().objects()){
						w.setVisibleToUser(mainThreadState.isAppForeground && w.getNativeLayer()->isVisible, getMonotonicTime());
					}
				} else {
					THROW() << "No data on command pipe!";
				}
				mainThreadState.commandCompletedSemaphore.post();
				break;
			}
			case APP_CMD_KEYUPDOWN:{
				KeyUpDownMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					KeyMessage::ExtraData extra;
					extra.keycode = msg.keycode;
					extra.modifiers = rhfw::KeyModifiers::NO_FLAG;
					KeyMessage::postMessage(msg.window, InputDevice::VIRTUAL_KEYBOARD, KeyAction::DOWN, extra);
					KeyMessage::postMessage(msg.window, InputDevice::VIRTUAL_KEYBOARD, KeyAction::UP, extra);
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_UNICODE_CHAR: {
				UnicodeCharMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					KeyMessage::ExtraData extra;
					extra.repeatUnicode = msg.codepoint;
					extra.repeatUnicodeCount = 1;
					KeyMessage::postMessage(msg.window, InputDevice::VIRTUAL_KEYBOARD, KeyAction::UNICODE_REPEAT, extra);
				} else {
					THROW() << "No data on command pipe!";
				}
				break;
			}
			case APP_CMD_UNICODE_SEQUENCE: {
				UnicodeSequenceMessage msg;
				if (read(fd, &msg, sizeof(msg)) == sizeof(msg)) {
					KeyMessage::ExtraData extra;
					extra.sequenceUnicode = msg.codepoints;
					extra.sequenceUnicodeCount = msg.length;
					KeyMessage::postMessage(msg.window, InputDevice::VIRTUAL_KEYBOARD, KeyAction::UNICODE_SEQUENCE, extra);
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

void platform_bridge::application_main(void* arg){
	initDpi();
	StorageDirectoryDescriptor::initializeRootDirectory(FilePath { getenv("HOME") } + "Library" + "appdata");
	StorageDirectoryDescriptor { StorageDirectoryDescriptor::Root() }.create();
	Thread::initApplicationMainThread();
	
	//::rhfw::module::initialize_modules();
	//TODO a snake eseten nem a default frame buffer-be ir, mivel annak a neve initstatic-ban nem megfeleloen van kiolvasva
	user_app_initialize(0, nullptr);
	
	int res;
	struct pollfd polldata;
	polldata.fd = mainThreadState.getPipeReadFd();
	polldata.events = POLLIN;
	
	while(true){
		core::GlobalMonotonicTimeListener::setCurrent(getMonotonicTime());
		
		//TODO tracking waiting times
		const bool didDraw = platform_bridge::executeDrawing();

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
#define INIT_FALLBACK_DPI() LOGWTF() << "Unknown device type: " << name; dpi = Size2F { 290, 290 }
void platform_bridge::initDpi(){
	struct utsname sysinfo;
	uname(&sysinfo);
	auto* name = sysinfo.machine;
	if(name[0] == 'i' && name[1] == 'P'){
		switch (name[2]) {
			case 'h':
				//iPhone
				switch (name[6]) {
					case '1':
						//iPhone1,
						//iPhone (First)
						//iPhone 3G
						dpi = {163, 163};
						break;
					case '2':
						//iPhone2,
						//iPhone 3GS
						dpi = {163, 163};
						break;
					case '3':
						//iPhone3,
						//iPhone 4
						dpi = {326, 326};
						break;
					case '4':
						//iPhone4,
						//iPhone 4S
						dpi = {326, 326};
						break;
					case '5':
						//iPhone5,
						//iPhone 5
						//iPhone 5C
						dpi = {326, 326};
						break;
					case '6':
						//iPhone6,
						//iPhone 5S
						dpi = {326, 326};
						break;
					case '7':
						//iPhone7,
						switch (name[8]) {
							case '1':
								//iPhone 6+
								dpi = {401, 401};
								break;
							case '2':
								//iPhone 6
								dpi = {326, 326};
								break;
							default:
								INIT_FALLBACK_DPI();
								break;
						}
						break;
					case '8':
						switch (name[8]) {
							case '1':
								//iPhone 6S
								dpi = {326, 326};
								break;
							case '2':
								//iPhone 6S+
								dpi = {401, 401};
								break;
							case '4':
								//iPhone SE
								dpi = {326, 326};
								break;
							default:
								INIT_FALLBACK_DPI();
								break;
						}
						break;
					default:
						INIT_FALLBACK_DPI();
						break;
				}
				break;
			case 'o':
				//iPod
				switch (name[4]) {
					case '1':
						//iPod 1gen
						dpi = {163, 163};
						break;
					case '2':
						//iPod 2gen
						dpi = {163, 163};
						break;
					case '3':
						//iPod 3gen
						dpi = {163, 163};
						break;
					case '4':
						//iPod 4gen
						dpi = {326, 326};
						break;
					case '5':
						//iPod 5gen
						dpi = {326, 326};
						break;
					case '7':
						//iPod 6gen
						dpi = {326, 326};
						break;
					default:
						INIT_FALLBACK_DPI();
						break;
				}
				break;
			case 'a':
				//iPad
				switch (name[4]) {
					case '1':
						//iPad 1gen
						dpi = {132, 132};
						break;
					case '2':
						switch (name[6]) {
							case '1':
							case '2':
							case '3':
							case '4':
								//iPad 2
								dpi = {132, 132};
								break;
							case '5':
							case '6':
							case '7':
								//iPad mini
								dpi = {163, 163};
								break;
							default:
								INIT_FALLBACK_DPI();
								break;
						}
						break;
					case '3':
						//iPad 3gen
						//ipad 4gen
						dpi = {264, 264};
						break;
					case '4':
						switch (name[6]) {
							case '1':
							case '2':
							case '3':
								//iPad Air
								dpi = {264, 264};
								break;
							case '4':
							case '5':
							case '6':
								//iPad mini 3
							case '7':
							case '8':
							case '9':
								//iPad mini 3
								dpi = {326, 326};
								break;
							default:
								INIT_FALLBACK_DPI();
								break;
						}
						break;
					case '5':
						switch (name[6]) {
							case '1':
							case '2':
								//iPad mini 4
								dpi = {326, 326};
								break;
							case '3':
							case '4':
								//iPad Air 2
								dpi = {264, 264};
								break;
							default:
								INIT_FALLBACK_DPI();
								break;
						}
						break;
					case '7':
						//iPad Pro
						dpi = {264, 264};
						break;
					default:
						INIT_FALLBACK_DPI();
						break;
				}
				break;
			default:
				INIT_FALLBACK_DPI();
				break;
		}
	}else{
		INIT_FALLBACK_DPI();
	}
}
/*
void applyPlatformConfigurations(ResourceConfiguration& config){
	//TODO
}
*/
}

int main(int argc, char * argv[]) {
	signal(SIGPIPE, SIG_IGN);
	@autoreleasepool {
		return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
	}
}
