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
#include <Windows.h>
#include <windowsx.h>
#include <framework/core/Window.h>
#include <framework/core/timing.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <gen/log.h>
#include <gen/configuration.h>
#include <framework/core/AppInterface.h>
#include <framework/threading/Thread.h>
#include <framework/threading/Semaphore.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/io/key/KeyModifierTracker.h>
#include <win32platform/window/Win32Platform.h>
#include <win32platform/window/MessageDefinitions.h>
#include <ShellScalingApi.h>
#include WINDOWS_EXECPTION_HELPER_INCLUDE

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

static LARGE_INTEGER performancefrequency;
LONGLONG getMicroSeconds() {
	//as seen @ https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	time.QuadPart *= 1000000;
	time.QuadPart /= performancefrequency.QuadPart;
	return time.QuadPart;
}
static ::rhfw::core::time_micros getMonotonicTime() {
	return ::rhfw::core::time_micros { getMicroSeconds() };
}

static HCURSOR BuiltinArrowCursor = LoadCursor(NULL, IDC_ARROW);

namespace rhfw {

struct user_msg {
	HWND hwnd;
	core::Window* window;
	UINT message;
	WPARAM wparam;
	LPARAM lparam;

};

static const LONG USER_MSG_POOL_SIZE = 256;
static LONG messageIndex = 0;
static user_msg messagePool[USER_MSG_POOL_SIZE];

class platform_bridge {
public:
	static DWORD appThreadId;
	static HANDLE messagePoolSemaphore;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static void initRootPath();

	static void destroyWindow(core::Window* window) {
		core::Window::removeWindow(*window);
		delete window;
	}

	static void updateTime() {
		core::GlobalMonotonicTimeListener::setCurrent(getMonotonicTime());
	}

	static bool executeDrawing() {
		bool result = false;
		for (auto&& w : core::Window::windows.objects()) {
			if (w.isReadyToDraw()) {
				w.draw();
				result = true;
			}
		}
		return result;
	}

	struct thread_args {
		Semaphore sem { Semaphore::auto_init { } };
	};
	static HANDLE startApplicationThread(HINSTANCE hInstance, int showcmd, int argc, char* argv[]);
	static DWORD WINAPI applicationThreadFunc(void*);

	static user_msg* getUserMsg() {
		DWORD res = WaitForSingleObject(messagePoolSemaphore, INFINITE);
		ASSERT(res == WAIT_OBJECT_0) << GetLastError();
		LONG index = messageIndex++;
		if (messageIndex >= USER_MSG_POOL_SIZE) {
			messageIndex = 0;
		}
		return messagePool + index;
	}
	static void releaseUserMsg() {
		BOOL res = ReleaseSemaphore(messagePoolSemaphore, 1, nullptr);
		ASSERT(res) << GetLastError();
	}
};
DWORD platform_bridge::appThreadId = 0;
static HINSTANCE appInstance = nullptr;
HANDLE platform_bridge::messagePoolSemaphore = NULL;

namespace win32platform {

static DWORD mainThreadId = 0;
HINSTANCE getApplicationInstance() {
	return appInstance;
}
DWORD getMainThreadId() {
	return mainThreadId;
}

} // namespace win32

/*
 void applyPlatformConfigurations(ResourceConfiguration& config) {
 //TODO
 }
 */
static const KeyCode KEYCODE_MAP[256] = {
/*   	0	: -              */KeyCode::KEY_UNKNOWN,
/*   	1	: LBUTTON        */KeyCode::KEY_UNKNOWN,
/*   	2	: RBUTTON        */KeyCode::KEY_UNKNOWN,
/*   	3	: CANCEL         */KeyCode::KEY_UNKNOWN,
/*   	4	: MBUTTON        */KeyCode::KEY_UNKNOWN,
/*   	5	: XBUTTON1       */KeyCode::KEY_UNKNOWN,
/*   	6	: XBUTTON2       */KeyCode::KEY_UNKNOWN,
/*   	7	: -              */KeyCode::KEY_UNKNOWN,
/*   	8	: BACK           */KeyCode::KEY_BACKSPACE,
/*   	9	: TAB            */KeyCode::KEY_TAB,
/*   	10	: -              */KeyCode::KEY_UNKNOWN,
/*   	11	: -              */KeyCode::KEY_UNKNOWN,
/*   	12	: CLEAR          */KeyCode::KEY_CLEAR,
/*   	13	: RETURN         */KeyCode::KEY_ENTER,
/*   	14	: -              */KeyCode::KEY_UNKNOWN,
/*   	15	: -              */KeyCode::KEY_UNKNOWN,
/*   	16	: SHIFT          */KeyCode::KEY_SHIFT,
/*   	17	: CONTROL        */KeyCode::KEY_CTRL,
/*   	18	: MENU           */KeyCode::KEY_ALT,
/*   	19	: PAUSE          */KeyCode::KEY_PAUSEBREAK,
/*   	20	: CAPITAL        */KeyCode::KEY_CAPSLOCK,
/*   	21	: KANA           */KeyCode::KEY_UNKNOWN,
/*   	22	: -              */KeyCode::KEY_UNKNOWN,
/*   	23	: JUNJA          */KeyCode::KEY_UNKNOWN,
/*   	24	: FINAL          */KeyCode::KEY_UNKNOWN,
/*   	25	: HANJA          */KeyCode::KEY_UNKNOWN,
/*   	26	: -              */KeyCode::KEY_UNKNOWN,
/*   	27	: ESCAPE         */KeyCode::KEY_ESC,
/*   	28	: CONVERT        */KeyCode::KEY_UNKNOWN,
/*   	29	: NONCONVERT     */KeyCode::KEY_UNKNOWN,
/*   	30	: ACCEPT         */KeyCode::KEY_UNKNOWN,
/*   	31	: MODECHANGE     */KeyCode::KEY_UNKNOWN,
/*   	32	: SPACE          */KeyCode::KEY_SPACE,
/*   	33	: PRIOR          */KeyCode::KEY_PAGE_UP,
/*   	34	: NEXT           */KeyCode::KEY_PAGE_DOWN,
/*   	35	: END            */KeyCode::KEY_END,
/*   	36	: HOME           */KeyCode::KEY_HOME,
/*   	37	: LEFT           */KeyCode::KEY_DIR_LEFT,
/*   	38	: UP             */KeyCode::KEY_DIR_UP,
/*   	39	: RIGHT          */KeyCode::KEY_DIR_RIGHT,
/*   	40	: DOWN           */KeyCode::KEY_DIR_DOWN,
/*   	41	: SELECT         */KeyCode::KEY_UNKNOWN,
/*   	42	: PRINT          */KeyCode::KEY_UNKNOWN,
/*   	43	: EXECUTE        */KeyCode::KEY_UNKNOWN,
/*   	44	: SNAPSHOT       */KeyCode::KEY_PRINTSCREEN,
/*   	45	: INSERT         */KeyCode::KEY_INSERT,
/*   	46	: DELETE         */KeyCode::KEY_DELETE,
/*   	47	: HELP           */KeyCode::KEY_UNKNOWN,
/*   	48	: Number0        */KeyCode::KEY_0,
/*   	49	: Number1        */KeyCode::KEY_1,
/*   	50	: Number2        */KeyCode::KEY_2,
/*   	51	: Number3        */KeyCode::KEY_3,
/*   	52	: Number4        */KeyCode::KEY_4,
/*   	53	: Number5        */KeyCode::KEY_5,
/*   	54	: Number6        */KeyCode::KEY_6,
/*   	55	: Number7        */KeyCode::KEY_7,
/*   	56	: Number8        */KeyCode::KEY_8,
/*   	57	: Number9        */KeyCode::KEY_9,
/*   	58	: -              */KeyCode::KEY_UNKNOWN,
/*   	59	: -              */KeyCode::KEY_UNKNOWN,
/*   	60	: -              */KeyCode::KEY_UNKNOWN,
/*   	61	: -              */KeyCode::KEY_UNKNOWN,
/*   	62	: -              */KeyCode::KEY_UNKNOWN,
/*   	63	: -              */KeyCode::KEY_UNKNOWN,
/*   	64	: -              */KeyCode::KEY_UNKNOWN,
/*   	65	: A              */KeyCode::KEY_A,
/*   	66	: B              */KeyCode::KEY_B,
/*   	67	: C              */KeyCode::KEY_C,
/*   	68	: D              */KeyCode::KEY_D,
/*   	69	: E              */KeyCode::KEY_E,
/*   	70	: F              */KeyCode::KEY_F,
/*   	71	: G              */KeyCode::KEY_G,
/*   	72	: H              */KeyCode::KEY_H,
/*   	73	: I              */KeyCode::KEY_I,
/*   	74	: J              */KeyCode::KEY_J,
/*   	75	: K              */KeyCode::KEY_K,
/*   	76	: L              */KeyCode::KEY_L,
/*   	77	: M              */KeyCode::KEY_M,
/*   	78	: N              */KeyCode::KEY_N,
/*   	79	: O              */KeyCode::KEY_O,
/*   	80	: P              */KeyCode::KEY_P,
/*   	81	: Q              */KeyCode::KEY_Q,
/*   	82	: R              */KeyCode::KEY_R,
/*   	83	: S              */KeyCode::KEY_S,
/*   	84	: T              */KeyCode::KEY_T,
/*   	85	: U              */KeyCode::KEY_U,
/*   	86	: V              */KeyCode::KEY_V,
/*   	87	: W              */KeyCode::KEY_W,
/*   	88	: X              */KeyCode::KEY_X,
/*   	89	: Y              */KeyCode::KEY_Y,
/*   	90	: Z              */KeyCode::KEY_Z,
/*   	91	: LWIN           */KeyCode::KEY_UNKNOWN,
/*   	92	: RWIN           */KeyCode::KEY_UNKNOWN,
/*   	93	: APPS           */KeyCode::KEY_UNKNOWN,
/*   	94	: -              */KeyCode::KEY_UNKNOWN,
/*   	95	: SLEEP          */KeyCode::KEY_UNKNOWN,
/*   	96	: NUMPAD0        */KeyCode::KEY_NUM_0,
/*   	97	: NUMPAD1        */KeyCode::KEY_NUM_1,
/*   	98	: NUMPAD2        */KeyCode::KEY_NUM_2,
/*   	99	: NUMPAD3        */KeyCode::KEY_NUM_3,
/*   	100	: NUMPAD4        */KeyCode::KEY_NUM_4,
/*   	101	: NUMPAD5        */KeyCode::KEY_NUM_5,
/*   	102	: NUMPAD6        */KeyCode::KEY_NUM_6,
/*   	103	: NUMPAD7        */KeyCode::KEY_NUM_7,
/*   	104	: NUMPAD8        */KeyCode::KEY_NUM_8,
/*   	105	: NUMPAD9        */KeyCode::KEY_NUM_9,
/*   	106	: MULTIP         */KeyCode::KEY_NUM_MULT,
/*   	107	: ADD            */KeyCode::KEY_NUM_ADD,
/*   	108	: SEPARATOR      */KeyCode::KEY_NUM_ENTER,
/*   	109	: SUBTRACT       */KeyCode::KEY_NUM_SUBTRACT,
/*   	110	: DECIMAL        */KeyCode::KEY_NUM_DOT,
/*   	111	: DIVIDE         */KeyCode::KEY_NUM_DIV,
/*   	112	: F1             */KeyCode::KEY_F1,
/*   	113	: F2             */KeyCode::KEY_F2,
/*   	114	: F3             */KeyCode::KEY_F3,
/*   	115	: F4             */KeyCode::KEY_F4,
/*   	116	: F5             */KeyCode::KEY_F5,
/*   	117	: F6             */KeyCode::KEY_F6,
/*   	118	: F7             */KeyCode::KEY_F7,
/*   	119	: F8             */KeyCode::KEY_F8,
/*   	120	: F9             */KeyCode::KEY_F9,
/*   	121	: F10            */KeyCode::KEY_F10,
/*   	122	: F11            */KeyCode::KEY_F11,
/*   	123	: F12            */KeyCode::KEY_F12,
/*   	124	: F13            */KeyCode::KEY_UNKNOWN,
/*   	125	: F14            */KeyCode::KEY_UNKNOWN,
/*   	126	: F15            */KeyCode::KEY_UNKNOWN,
/*   	127	: F16            */KeyCode::KEY_UNKNOWN,
/*   	128	: F17            */KeyCode::KEY_UNKNOWN,
/*   	129	: F18            */KeyCode::KEY_UNKNOWN,
/*   	130	: F19            */KeyCode::KEY_UNKNOWN,
/*   	131	: F20            */KeyCode::KEY_UNKNOWN,
/*   	132	: F21            */KeyCode::KEY_UNKNOWN,
/*   	133	: F22            */KeyCode::KEY_UNKNOWN,
/*   	134	: F23            */KeyCode::KEY_UNKNOWN,
/*   	135	: F24            */KeyCode::KEY_UNKNOWN,
/*   	136	: -              */KeyCode::KEY_UNKNOWN,
/*   	137	: -              */KeyCode::KEY_UNKNOWN,
/*   	138	: -              */KeyCode::KEY_UNKNOWN,
/*   	139	: -              */KeyCode::KEY_UNKNOWN,
/*   	140	: -              */KeyCode::KEY_UNKNOWN,
/*   	141	: -              */KeyCode::KEY_UNKNOWN,
/*   	142	: -              */KeyCode::KEY_UNKNOWN,
/*   	143	: -              */KeyCode::KEY_UNKNOWN,
/*   	144	: NUMLOCK        */KeyCode::KEY_NUMLOCK,
/*   	145	: SCROLL         */KeyCode::KEY_SCROLLLOCK,
/*   	146	: -              */KeyCode::KEY_UNKNOWN,
/*   	147	: -              */KeyCode::KEY_UNKNOWN,
/*   	148	: -              */KeyCode::KEY_UNKNOWN,
/*   	149	: -              */KeyCode::KEY_UNKNOWN,
/*   	150	: -              */KeyCode::KEY_UNKNOWN,
/*   	151	: -              */KeyCode::KEY_UNKNOWN,
/*   	152	: -              */KeyCode::KEY_UNKNOWN,
/*   	153	: -              */KeyCode::KEY_UNKNOWN,
/*   	154	: -              */KeyCode::KEY_UNKNOWN,
/*   	155	: -              */KeyCode::KEY_UNKNOWN,
/*   	156	: -              */KeyCode::KEY_UNKNOWN,
/*   	157	: -              */KeyCode::KEY_UNKNOWN,
/*   	158	: -              */KeyCode::KEY_UNKNOWN,
/*   	159	: -              */KeyCode::KEY_UNKNOWN,
/*   	160	: LSHIFT         */KeyCode::KEY_LEFT_SHIFT,
/*   	161	: RSHIFT         */KeyCode::KEY_RIGHT_SHIFT,
/*   	162	: LCONTROL       */KeyCode::KEY_LEFT_CTRL,
/*   	163	: RCONTROL       */KeyCode::KEY_RIGHT_CTRL,
/*   	164	: LMENU          */KeyCode::KEY_LEFT_ALT,
/*   	165	: RMENU          */KeyCode::KEY_RIGHT_ALT,
/*   	166	: BROWSER_BACK   */KeyCode::KEY_UNKNOWN,
/*   	167	: BROWSER_FORWARD*/KeyCode::KEY_UNKNOWN,
/*   	168	: BROWSER_REFRESH*/KeyCode::KEY_UNKNOWN,
/*   	169	: BROWSER_STOP   */KeyCode::KEY_UNKNOWN,
/*   	170	: BROWSER_SEARCH */KeyCode::KEY_UNKNOWN,
/*   	171	: BROWSER_FAVORIT*/KeyCode::KEY_UNKNOWN,
/*   	172	: BROWSER_HOME   */KeyCode::KEY_UNKNOWN,
/*   	173	: VOLUME_MUTE    */KeyCode::KEY_UNKNOWN,
/*   	174	: VOLUME_DOWN    */KeyCode::KEY_UNKNOWN,
/*   	175	: VOLUME_UP      */KeyCode::KEY_UNKNOWN,
/*   	176	: MEDIA_NEXT_TRAC*/KeyCode::KEY_UNKNOWN,
/*   	177	: MEDIA_PREV_TRAC*/KeyCode::KEY_UNKNOWN,
/*   	178	: MEDIA_STOP     */KeyCode::KEY_UNKNOWN,
/*   	179	: MEDIA_PLAY_PAUS*/KeyCode::KEY_UNKNOWN,
/*   	180	: LAUNCH_MAIL    */KeyCode::KEY_UNKNOWN,
/*   	181	: LAUNCH_MEDIA_SE*/KeyCode::KEY_UNKNOWN,
/*   	182	: LAUNCH_APP1    */KeyCode::KEY_UNKNOWN,
/*   	183	: LAUNCH_APP2    */KeyCode::KEY_UNKNOWN,
/*   	184	: -              */KeyCode::KEY_UNKNOWN,
/*   	185	: -              */KeyCode::KEY_UNKNOWN,
/*   	186	: OEM_1          */KeyCode::KEY_UNKNOWN,
/*   	187	: OEM_PLUS       */KeyCode::KEY_UNKNOWN,
/*   	188	: OEM_COMMA      */KeyCode::KEY_UNKNOWN,
/*   	189	: OEM_MINUS      */KeyCode::KEY_UNKNOWN,
/*   	190	: OEM_PERIOD     */KeyCode::KEY_UNKNOWN,
/*   	191	: OEM_2          */KeyCode::KEY_UNKNOWN,
/*   	192	: OEM_3          */KeyCode::KEY_UNKNOWN,
/*   	193	: -              */KeyCode::KEY_UNKNOWN,
/*   	194	: -              */KeyCode::KEY_UNKNOWN,
/*   	195	:                */KeyCode::KEY_UNKNOWN,
/*   	196	:                */KeyCode::KEY_UNKNOWN,
/*   	197	:                */KeyCode::KEY_UNKNOWN,
/*   	198	:                */KeyCode::KEY_UNKNOWN,
/*   	199	:                */KeyCode::KEY_UNKNOWN,
/*   	200	:                */KeyCode::KEY_UNKNOWN,
/*   	201	:                */KeyCode::KEY_UNKNOWN,
/*   	202	:                */KeyCode::KEY_UNKNOWN,
/*   	203	:                */KeyCode::KEY_UNKNOWN,
/*   	204	:                */KeyCode::KEY_UNKNOWN,
/*   	205	:                */KeyCode::KEY_UNKNOWN,
/*   	206	:                */KeyCode::KEY_UNKNOWN,
/*   	207	:                */KeyCode::KEY_UNKNOWN,
/*   	208	:                */KeyCode::KEY_UNKNOWN,
/*   	209	:                */KeyCode::KEY_UNKNOWN,
/*   	210	:                */KeyCode::KEY_UNKNOWN,
/*   	211	:                */KeyCode::KEY_UNKNOWN,
/*   	212	:                */KeyCode::KEY_UNKNOWN,
/*   	213	:                */KeyCode::KEY_UNKNOWN,
/*   	214	:                */KeyCode::KEY_UNKNOWN,
/*   	215	:                */KeyCode::KEY_UNKNOWN,
/*   	216	:                */KeyCode::KEY_UNKNOWN,
/*   	217	:                */KeyCode::KEY_UNKNOWN,
/*   	218	:                */KeyCode::KEY_UNKNOWN,
/*   	219	: OEM_4          */KeyCode::KEY_UNKNOWN,
/*   	220	: OEM_5          */KeyCode::KEY_UNKNOWN,
/*   	221	: OEM_6          */KeyCode::KEY_UNKNOWN,
/*   	222	: OEM_7          */KeyCode::KEY_UNKNOWN,
/*   	223	: OEM_8          */KeyCode::KEY_UNKNOWN,
/*   	224	: -              */KeyCode::KEY_UNKNOWN,
/*   	225	: -              */KeyCode::KEY_UNKNOWN,
/*   	226	: OEM_102        */KeyCode::KEY_UNKNOWN,
/*   	227	: -              */KeyCode::KEY_UNKNOWN,
/*   	228	: -              */KeyCode::KEY_UNKNOWN,
/*   	229	: PROCESSKEY     */KeyCode::KEY_UNKNOWN,
/*   	230	: -              */KeyCode::KEY_UNKNOWN,
/*   	231	: PACKET         */KeyCode::KEY_UNKNOWN,
/*   	232	: -              */KeyCode::KEY_UNKNOWN,
/*   	233	: -              */KeyCode::KEY_UNKNOWN,
/*   	234	: -              */KeyCode::KEY_UNKNOWN,
/*   	235	: -              */KeyCode::KEY_UNKNOWN,
/*   	236	: -              */KeyCode::KEY_UNKNOWN,
/*   	237	: -              */KeyCode::KEY_UNKNOWN,
/*   	238	: -              */KeyCode::KEY_UNKNOWN,
/*   	239	: -              */KeyCode::KEY_UNKNOWN,
/*   	240	: -              */KeyCode::KEY_UNKNOWN,
/*   	241	: -              */KeyCode::KEY_UNKNOWN,
/*   	242	: -              */KeyCode::KEY_UNKNOWN,
/*   	243	: -              */KeyCode::KEY_UNKNOWN,
/*   	244	: -              */KeyCode::KEY_UNKNOWN,
/*   	245	: -              */KeyCode::KEY_UNKNOWN,
/*   	246	: ATTN           */KeyCode::KEY_UNKNOWN,
/*   	247	: CRSEL          */KeyCode::KEY_UNKNOWN,
/*   	248	: EXSEL          */KeyCode::KEY_UNKNOWN,
/*   	249	: EREOF          */KeyCode::KEY_UNKNOWN,
/*   	250	: PLAY           */KeyCode::KEY_UNKNOWN,
/*   	251	: ZOOM           */KeyCode::KEY_UNKNOWN,
/*   	252	: NONAME         */KeyCode::KEY_UNKNOWN,
/*   	253	: PA1            */KeyCode::KEY_UNKNOWN,
/*   	254	: OEM_CLEAR      */KeyCode::KEY_UNKNOWN,
/*   	255	: -              */KeyCode::KEY_UNKNOWN, };

static KeyCode ConvertKeyCode(WPARAM wParam, LPARAM lParam) {
	UINT vk;
	switch (wParam) {
		case VK_SHIFT: {
			vk = MapVirtualKey((lParam & 0x01FF0000) >> 16, MAPVK_VSC_TO_VK_EX);
			ASSERT(vk != 0);
			break;
		}
		case VK_CONTROL: {
			if (lParam & 0x1000000) {
				vk = VK_RCONTROL;
			} else {
				vk = VK_LCONTROL;
			}
			break;
		}
		case VK_MENU: {
			if (lParam & 0x1000000) {
				vk = VK_RMENU;
			} else {
				vk = VK_LMENU;
			}
			break;
		}
		default: {
			vk = wParam;
			break;
		}
	}
	//0-255
	auto res = KEYCODE_MAP[(unsigned char) vk];
	WARN(res == KeyCode::KEY_UNKNOWN) << "Unknown keycode: " << (unsigned int) (unsigned char) wParam << " vk: " << vk;
	LOGI()<< "Scancode: " << ((lParam & 0x01FF0000) >> 16) << " -> " << res;
	return res;
}

} // namespace rhfw;

static void registerWindowClass(HINSTANCE appinstance) {
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = ::rhfw::platform_bridge::WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = appinstance;
	wc.hCursor = BuiltinArrowCursor;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WIN32WINDOW_CLASSNAME;
	wc.hIcon = LoadIcon(appinstance, "APPICON");
	wc.hIconSm = NULL;

	auto regres = RegisterClassEx(&wc);
	ASSERT(regres != 0) << GetLastError();
}

class WindowCreationArgs {
public:
	::rhfw::core::Window* window;
	bool recreating;

	WindowCreationArgs(::rhfw::core::Window* window, bool recreating = false)
			: window { window }, recreating { recreating } {
	}
};

typedef HRESULT (WINAPI *GetDpiForMonitorFunctionType)( _In_ HMONITOR hmonitor, _In_ MONITOR_DPI_TYPE dpiType, _Out_ UINT *dpiX,
		_Out_ UINT *dpiY);
typedef HRESULT (WINAPI *SetProcessDpiAwarenessType)( _In_ PROCESS_DPI_AWARENESS value);
static GetDpiForMonitorFunctionType getDpiForMonitorFunction = nullptr;
static SetProcessDpiAwarenessType setProcessDpiAwarenessFunction = nullptr;

static rhfw::Semaphore main_appthread_communication_semaphore { rhfw::Semaphore::auto_init { } };

class ProgramArguments {
public:
	int argc;
	char** argv;
};
HANDLE rhfw::platform_bridge::startApplicationThread(HINSTANCE hInstance, int showcmd, int argc, char* argv[]) {
	appInstance = hInstance;
	messagePoolSemaphore = CreateSemaphore(nullptr, USER_MSG_POOL_SIZE, USER_MSG_POOL_SIZE, nullptr);

	ProgramArguments pa { argc, argv };

	HANDLE threadhandle = CreateThread(nullptr, 0, applicationThreadFunc, &pa, 0, nullptr);
	ASSERT(threadhandle != NULL) << GetLastError();
	appThreadId = GetThreadId(threadhandle);
	main_appthread_communication_semaphore.wait();

	core::Window* window = new core::Window { WindowStyle::BORDERED };
	WindowCreationArgs createargs { window };

	HWND hwnd = CreateWindowEx(0,	//extended style
			WIN32WINDOW_CLASSNAME,    // name of the window class
			APPLICATION_DEFAULT_NAME,   // title of the window
			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU,    // window style
			CW_USEDEFAULT,    // x-position of the window
			CW_USEDEFAULT,    // y-position of the window
			640,    // width of the window
			480,    // height of the window
			NULL,    // no parent window, NULL
			NULL,    // not using menus, NULL
			hInstance,    // application handle
			&createargs);    //
	ShowWindowAsync(hwnd, showcmd);

	return threadhandle;
}

static rhfw::KeyModifierTracker modifier_states;

DWORD WINAPI rhfw::platform_bridge::applicationThreadFunc(LPVOID arg) {
	ProgramArguments pa = *reinterpret_cast<ProgramArguments*>(arg);
	ThrowIfFailed(CoInitialize(nullptr));

	HMODULE shcoredll = LoadLibraryA("Shcore.dll");
	WARN(shcoredll == NULL) << "Shcore.dll not found";
	if (shcoredll != NULL) {
		getDpiForMonitorFunction = (GetDpiForMonitorFunctionType) GetProcAddress(shcoredll, "GetDpiForMonitor");
		setProcessDpiAwarenessFunction = (SetProcessDpiAwarenessType) GetProcAddress(shcoredll, "SetProcessDpiAwareness");
		WARN(getDpiForMonitorFunction == nullptr) << "GetDpiForMonitor not found";
		WARN(setProcessDpiAwarenessFunction == nullptr) << "SetProcessDpiAwareness not found";
		if (setProcessDpiAwarenessFunction != nullptr) {
			HRESULT res = setProcessDpiAwarenessFunction(PROCESS_PER_MONITOR_DPI_AWARE);
			ASSERT(SUCCEEDED(res));
		}
	}

	MSG msg;
	//create message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	main_appthread_communication_semaphore.post();

	Thread::initApplicationMainThread();
	StorageDirectoryDescriptor::InitializePlatformRootDirectory(pa.argc, pa.argv);

	updateTime();

	user_app_initialize(pa.argc, pa.argv);

	while (true) {
		::rhfw::platform_bridge::updateTime();
		::rhfw::platform_bridge::executeDrawing();

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			::rhfw::platform_bridge::updateTime();
			//sometimes, WM_USER message comes from nowhere
			//range check, to protect app from malicious messages
			if (msg.message == WM_USER_MESSAGE && (user_msg*) msg.lParam >= messagePool
					&& (user_msg*) msg.lParam < messagePool + USER_MSG_POOL_SIZE) {
				user_msg data = *(user_msg*) msg.lParam;
				releaseUserMsg();
				core::Window* window = data.window;
				switch (data.message) {
					case WM_CREATE: {
						HWND oldhwnd = window->updateHwnd(data.hwnd);
						main_appthread_communication_semaphore.post();
						window->finishHwndUpdate(oldhwnd, core::MonotonicTime::getCurrent());
						core::Window::addWindow(*window, nullptr);
						break;
					}
					case WM_SHOWWINDOW: {
						window->setVisibleToUser(data.wparam != FALSE, core::MonotonicTime::getCurrent());
						break;
					}
					case WM_SIZE: {
						UINT dpix = 96;
						UINT dpiy = 96;
						//TODO should be MDT_EFFECTIVE_DPI ?
						if (getDpiForMonitorFunction != nullptr) {
							UINT x;
							UINT y;
							HMONITOR monitor = MonitorFromWindow(window->getHwnd(), MONITOR_DEFAULTTONEAREST);
							HRESULT res = getDpiForMonitorFunction(monitor, MDT_RAW_DPI, &x, &y);
							if (SUCCEEDED(res)) {
								dpix = x;
								dpiy = y;
							}
						}
						core::WindowSize size;
						size.pixelSize = Size2UI { LOWORD(data.lparam), HIWORD(data.lparam) };
						size.dpi = Size2UI { dpix, dpiy };
						window->setSize(size);
						if ((int) data.wparam >= 0 && data.wparam != SIZE_MAXHIDE && data.wparam != SIZE_MAXSHOW) {
							if (data.wparam == SIZE_MAXIMIZED) {
								window->windowStyle |= WindowStyle::FULLSCREEN;
							} else {
								window->windowStyle &= ~WindowStyle::FULLSCREEN;
							}
						}
						break;
					}
					case WM_LBUTTONDOWN: {
						TouchMessage::postMessage(window, InputDevice::MOUSE, 0, TouchAction::DOWN, (float) GET_X_LPARAM(data.lparam),
								(float) GET_Y_LPARAM(data.lparam), getMonotonicTime());
						break;
					}
					case WM_LBUTTONUP: {
						TouchMessage::postMessage(window, InputDevice::MOUSE, 0, TouchAction::UP, (float) GET_X_LPARAM(data.lparam),
								(float) GET_Y_LPARAM(data.lparam), getMonotonicTime());
						break;
					}
					case WM_RBUTTONDOWN: {
						break;
					}
					case WM_RBUTTONUP: {
						break;
					}
					case WM_MOUSEMOVE: {
						if ((data.wparam & MK_LBUTTON) == MK_LBUTTON) {
							TouchMessage::postMessage(window, InputDevice::MOUSE, 0, TouchAction::MOVED_SINGLE,
									(float) GET_X_LPARAM(data.lparam), (float) GET_Y_LPARAM(data.lparam), getMonotonicTime());
						} else {
							TouchMessage::postMessage(window, InputDevice::MOUSE, 0, TouchAction::HOVER_MOVE,
									(float) GET_X_LPARAM(data.lparam), (float) GET_Y_LPARAM(data.lparam), getMonotonicTime());
						}
						break;
					}
					case WM_MOUSEWHEEL: {
						LOGTRACE()<< "WM_MOUSEWHEEL: " << GET_WHEEL_DELTA_WPARAM(data.wparam) << " - " << (float) GET_X_LPARAM(data.lparam) << ", " << (float) GET_Y_LPARAM(data.lparam);
						POINT p;
						p.x = GET_X_LPARAM(data.lparam);
						p.y = GET_Y_LPARAM(data.lparam);
						ScreenToClient(data.hwnd, &p);

						TouchEvent::ExtraData extra;
						extra.wheel = GET_WHEEL_DELTA_WPARAM(data.wparam);
						extra.wheelMax = WHEEL_DELTA;
						TouchMessage::postMessage(window, InputDevice::MOUSE, 0, TouchAction::WHEEL,
								p.x, p.y, getMonotonicTime(), extra);
						break;
					}
					case WM_KEYDOWN: {
						KeyMessage::ExtraData extra;
						extra.repeat = HAS_FLAG(data.lparam, 0x40000000);
						extra.keycode = ConvertKeyCode(data.wparam, data.lparam);
						modifier_states.down(extra.keycode);
						extra.modifiers = modifier_states;
						KeyMessage::postMessage(window, InputDevice::KEYBOARD, KeyAction::DOWN, extra);
						break;
					}
					case WM_KEYUP: {
						KeyMessage::ExtraData extra;
						extra.keycode = ConvertKeyCode(data.wparam, data.lparam);
						modifier_states.up(extra.keycode);
						extra.modifiers = modifier_states;
						KeyMessage::postMessage(window, InputDevice::KEYBOARD, KeyAction::UP, extra);
						break;
					}
					case WM_UNICHAR: {
						KeyMessage::ExtraData extra;
						extra.repeatUnicode = (uint32) data.wparam;
						extra.repeatUnicodeCount = 1;
						KeyMessage::postMessage(window, InputDevice::KEYBOARD, KeyAction::UNICODE_REPEAT, extra);
						break;
					}
					case WM_DESTROY:
					case WM_CLOSE: {
						main_appthread_communication_semaphore.post();
						destroyWindow(window);
						if (!core::Window::hasWindows()) {
							BOOL bres = PostThreadMessage(win32platform::getMainThreadId(), WM_QUIT, 0, 0);
							ASSERT(bres) << GetLastError();
						}
						break;
					}
					case WM_SETFOCUS: {
						window->setInputFocused(true);
						break;
					}
					case WM_KILLFOCUS: {
						window->setInputFocused(false);
						break;
					}
					default: {
						break;
					}
				}
			} else {
				DispatchMessage(&msg);
			}
		}
		if (msg.message == WM_QUIT)
			break;
	}
	user_app_terminate();
	if (shcoredll != NULL) {
		FreeLibrary(shcoredll);
	}
	CoUninitialize();
	LOGTRACE()<< "Exit app thread";
	return 0;
}
int main(int argc, char* argv[]) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
	int nCmdShow = SW_SHOWDEFAULT;
	rhfw::win32platform::mainThreadId = GetCurrentThreadId();
#if LOGGING_ENABLED
	//for logging
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif /* RHFW_DEBUG */

	//START_TRACK_MEMORY_LEAKS();
	registerWindowClass(hInstance);
	QueryPerformanceFrequency(&performancefrequency);

	HANDLE appthread = ::rhfw::platform_bridge::startApplicationThread(hInstance, nCmdShow, argc, argv);

	MSG msg;

	for (BOOL ret; (ret = GetMessage(&msg, NULL, 0, 0)) != 0;) {
		if (ret == -1) {
			//some error happened
			THROW() << "GetMessage returned -1. Last error: " << GetLastError();
			break;
		}
		if (msg.message == WM_USER_EXECUTE) {
			(*((WinMessageRunnable*) msg.lParam))();
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_QUIT, 0, 0);
	DWORD res = WaitForSingleObject(appthread, INFINITE);
	ASSERT(res == WAIT_OBJECT_0) << GetLastError();
	CloseHandle(appthread);
	CloseHandle(::rhfw::platform_bridge::messagePoolSemaphore);

	LOG_MEMORY_LEAKS();
	LOGTRACE()<< "Return main: " << msg.wParam;
	return msg.wParam;
}
//int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
//}

static BOOL transformClientRectToScreen(HWND hwnd, RECT* inoutrect) {
	POINT clientpos { 0, 0 };

	BOOL res = ClientToScreen(hwnd, &clientpos);
	ASSERT(res) << GetLastError();
	if (!res) {
		return FALSE;
	}

	inoutrect->left += clientpos.x;
	inoutrect->right += clientpos.x;
	inoutrect->top += clientpos.y;
	inoutrect->bottom += clientpos.y;

	return TRUE;
}

static BOOL getHwndClientRect(HWND hwnd, RECT* outrect) {
	BOOL res;
	POINT cursorpos;
	res = GetClientRect(hwnd, outrect);
	ASSERT(res) << GetLastError();
	if (!res) {
		return FALSE;
	}

	return transformClientRectToScreen(hwnd, outrect);
}

LRESULT CALLBACK ::rhfw::platform_bridge::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE: {
			WindowCreationArgs* createargs = (WindowCreationArgs*) reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) createargs->window);
			user_msg* data = getUserMsg();
			data->window = createargs->window;
			data->hwnd = hwnd;
			data->message = message;
			data->wparam = wParam;
			data->lparam = lParam;

			BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
			ASSERT(bres) << GetLastError();
			main_appthread_communication_semaphore.wait();
			return 0;
		}
		case WM_SIZE: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				if(window->clipCursor) {
					BOOL res;
					RECT cr;
					res = getHwndClientRect(hwnd, &cr);
					ASSERT(res) << GetLastError();
					res = ClipCursor(&cr);
					ASSERT(res) << GetLastError();
				}
				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
			}
			break;
		}
		case WM_SHOWWINDOW: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
			}
			break;
		}
		case WM_WINDOWPOSCHANGED: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				RECT rect;
				GetClientRect(hwnd, &rect);

				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = WM_SIZE;
				data->wparam = -1;
				data->lparam = MAKELONG(rect.right, rect.bottom);
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();

				if(window->clipCursor && transformClientRectToScreen(hwnd, &rect)) {
					BOOL res;
					res = ClipCursor(&rect);
					ASSERT(res) << GetLastError();
				}
			}
			break;
		}
		case WM_USER_DESTROY_WINDOW: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			//send destroy message to app thread first
			//to avoid deadlock with messages sent back to main thread (for example SetFullscreenState)
			if (window != nullptr) {
				SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);

				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = WM_DESTROY;
				data->wparam = 0;
				data->lparam = 0;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				main_appthread_communication_semaphore.wait();

				BOOL res = DestroyWindow(hwnd);
				ASSERT(res) << GetLastError();
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);

				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = 0;
				data->lparam = 0;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				main_appthread_communication_semaphore.wait();

				//make sure app thread is notified, and no further rendering will be done
			}
			break;
		}
		case WM_SETFOCUS: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr ) {
				if(window->clipCursor) {
					BOOL res;
					RECT cr;
					res = getHwndClientRect(hwnd, &cr);
					ASSERT(res) << GetLastError();
					res = ClipCursor(&cr);
					ASSERT(res) << GetLastError();
				}

				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				return 0;
			}
			break;
		}
		case WM_KILLFOCUS: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr ) {
				if(window->clipCursor) {
					ClipCursor(NULL);
				}

				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				return 0;
			}
			break;
		}
		case WM_MOUSEWHEEL:
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
//		case WM_RBUTTONDOWN:
//		case WM_RBUTTONUP:
		case WM_MOUSEMOVE: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = message;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				return 0;
			}
			break;
		}
		case WM_SYSKEYDOWN: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = WM_KEYDOWN;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				return 0;
			}
			break;
		}
		case WM_SYSKEYUP: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				user_msg* data = getUserMsg();
				data->window = window;
				data->hwnd = hwnd;
				data->message = WM_KEYUP;
				data->wparam = wParam;
				data->lparam = lParam;
				BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
				ASSERT(bres) << GetLastError();
				return 0;
			}
			break;
		}
		case WM_SYSCOMMAND: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				switch (wParam) {
					case SC_KEYMENU: {
						return 0;
					}
					case SC_RESTORE: {
						if (HAS_FLAG(window->appliedWindowStyle,
										WindowStyle::FULLSCREEN) && !HAS_FLAG(window->appliedWindowStyle, WindowStyle::BORDERED)) {
							ShowWindow(hwnd, SW_MAXIMIZE);
							return 0;
						}
						break;
					}
					default: {
						break;
					}
				}
			}
			break;
		}
		/*case WM_POINTERLEAVE: {
		 return 1;
		 }*/
		case WM_PAINT: {
			return 0;
		}
		case WM_USER_RECREATE_WINDOW: {
			LOGTRACE() << "WM_RECREATE " << hwnd;
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
			LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
			LONG extstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
			HWND parent = GetParent(hwnd);
			HWND focusedhwnd = GetFocus();

			WINDOWPLACEMENT placement;
			placement.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hwnd, &placement);
			RECT wrect = placement.rcNormalPosition;
			WindowCreationArgs createargs {window, true};
			HWND nhwnd = CreateWindowEx(extstyle, WIN32WINDOW_CLASSNAME, window->getWindowTitle(), style, wrect.left, wrect.top,
					wrect.right - wrect.left, wrect.bottom - wrect.top, parent, NULL, win32platform::getApplicationInstance(), &createargs);
			ShowWindow(nhwnd, placement.showCmd);

			if (focusedhwnd == hwnd) {
				SetFocus(nhwnd);
			}
			DestroyWindow(hwnd);
			return 0;
		}
		/*case WM_USER_EXECUTE: {
		 (*((WinMessageRunnable*) lParam))();
		 break;
		 }*/
		case WM_GETMINMAXINFO: {
			MINMAXINFO* mmi = (MINMAXINFO*) lParam;
			mmi->ptMinTrackSize.x = 400;
			mmi->ptMinTrackSize.y = 400;
			return 0;
		}
		case WM_UNICHAR: {
			if (wParam == UNICODE_NOCHAR) {
				return TRUE;
			}
			break;
		}
		case WM_CHAR: {
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (window != nullptr) {
				UnicodeCodePoint codepoint = (uint32) wParam;
				if (IS_HIGH_SURROGATE(codepoint)) {
					window->lastHighSurrogate = codepoint;
				} else {
					if (IS_SURROGATE_PAIR(window->lastHighSurrogate, codepoint)) {
						codepoint = 0x10000 | ((window->lastHighSurrogate & 0x3FF) << 10) | (codepoint & 0x3FF);
						window->lastHighSurrogate = 0;
					}
					user_msg* data = getUserMsg();
					data->window = window;
					data->hwnd = hwnd;
					data->message = WM_UNICHAR;
					data->wparam = codepoint;
					data->lparam = lParam;
					BOOL bres = PostThreadMessage(::rhfw::platform_bridge::appThreadId, WM_USER_MESSAGE, 0, (LONG_PTR) data);
					ASSERT(bres) << GetLastError();
				}
				return 0;
			}
			break;
		}
		case WM_SETCURSOR: {
			if (LOWORD(lParam) == HTCLIENT) {
				core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if(window->isCursorVisible()) {
					break;
				} else {
					SetCursor(NULL);
					return TRUE;
				}
			}
			break;
		}
		case WM_USER_CURSOR_VISIBILITY: {
			bool visible = wParam != 0;
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if(window != nullptr && window->cursorVisible != visible) {
				window->cursorVisible = visible;
				BOOL res;
				RECT clientrect;
				POINT cursorpos;
				res = getHwndClientRect(hwnd, &clientrect);
				ASSERT(res) << GetLastError();

				res = GetCursorPos(&cursorpos);
				ASSERT(res) << GetLastError();

				if(cursorpos.x >= clientrect.left && cursorpos.x < clientrect.right &&
						cursorpos.y >= clientrect.top && cursorpos.y < clientrect.bottom) {
					if(visible) {
						SetCursor(BuiltinArrowCursor);
					} else {
						SetCursor(NULL);
					}
				}
			}
			break;
		}
		case WM_USER_CURSOR_CLIPPING: {
			bool clip = wParam != 0;
			core::Window* window = (core::Window*) (LONG_PTR) GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if(window != nullptr && window->clipCursor != clip) {
				if(GetForegroundWindow() == hwnd) {
					if(clip) {
						BOOL res;
						RECT cr;
						res = getHwndClientRect(hwnd, &cr);
						ASSERT(res) << GetLastError();
						res = ClipCursor(&cr);
						ASSERT(res) << GetLastError();
					} else {
						ClipCursor(NULL);
					}
				}
				window->clipCursor = clip;
			}
			break;
		}
		default: {
			break;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
