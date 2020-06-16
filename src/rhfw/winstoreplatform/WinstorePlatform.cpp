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
#include <winstoreplatform/WinstorePlatform.h>

#include <framework/core/timing.h>
#include <framework/resource/ResourceManager.h>
#include <framework/io/files/StorageDirectoryDescriptor.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/io/touch/TouchMessage.h>
#include <framework/io/key/KeyMessage.h>
#include <framework/threading/Thread.h>
#include <framework/core/Window.h>
#include <framework/core/AppInterface.h>
#include <framework/io/key/KeyModifierTracker.h>
#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>
//#include <gen/resconfig.h>
//#include <gen/modules.h>

#include <cstdarg>
#include <ppltasks.h>

namespace rhfw {

// Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
ref class App sealed : public ::Windows::ApplicationModel::Core::IFrameworkView {
private:
	void OnKeyDown(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::KeyEventArgs ^args);
	void OnKeyUp(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::KeyEventArgs ^args);
	void OnCharacterReceived(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::CharacterReceivedEventArgs ^args);

protected:
	// Application lifecycle event handlers.
	void OnActivated(::Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, ::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(::Platform::Object^ sender, ::Platform::Object^ args);

	// Window event handlers.
	void OnWindowSizeChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnWindowClosed(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::CoreWindowEventArgs^ args);

	// DisplayInformation event handlers.
	void OnDpiChanged(::Windows::Graphics::Display::DisplayInformation^ sender, ::Platform::Object^ args);
	void OnOrientationChanged(::Windows::Graphics::Display::DisplayInformation^ sender, ::Platform::Object^ args);
	void OnDisplayContentsInvalidated(::Windows::Graphics::Display::DisplayInformation^ sender, ::Platform::Object^ args);

	void OnPointerPressed(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerReleased(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerWheelChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args);

	void OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher^ sender, ::Windows::UI::Core::AcceleratorKeyEventArgs^ args);

	void BackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ e);

public:
	App() {
	}

	// IFrameworkView methods.
	virtual void Initialize(::Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(::Windows::UI::Core::CoreWindow^ window);
	virtual void Load(::Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();
};

ref class Direct3DApplicationSource sealed : ::Windows::ApplicationModel::Core::IFrameworkViewSource {
public:
	virtual ::Windows::ApplicationModel::Core::IFrameworkView^ CreateView() {
		return ref new App();
	}
};

static float getResolutionScale(::Windows::Graphics::Display::DisplayInformation^ info) {
	return (float)info->RawPixelsPerViewPixel;
}
static float getResolutionScale() {
	return getResolutionScale(::Windows::Graphics::Display::DisplayInformation::GetForCurrentView());
}

static LARGE_INTEGER performancefrequency;
static float resolutionScale;

static core::time_micros getMonotonicTime() {
	//as seen @ https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	time.QuadPart *= 1000000;
	time.QuadPart /= performancefrequency.QuadPart;

	return core::time_micros { time.QuadPart };
}

class platform_bridge {
public:
	static void updateTime() {
		core::GlobalMonotonicTimeListener::setCurrent(getMonotonicTime());
	}
	static void initMainThread() {
		Thread::initApplicationMainThread();
	}
	static core::Window* getWindowForCoreWindow(::Windows::UI::Core::CoreWindow^ window) {
		for (auto&& w : core::Window::windows.objects()) {
			if (w.getNativeWindow() == window) {
				return &w;
			}
		}
		return nullptr;
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

	static void changeWindowSize(core::Window* window, const ::Windows::Foundation::Rect& bounds, ::Windows::Graphics::Display::DisplayInformation^ dinfo) {
		float xdpi = dinfo->RawDpiX;
		float ydpi = dinfo->RawDpiY;
		if (xdpi == 0.0f || ydpi == 0.0f) {
			LOGW() << "DPI values are not available";
			//these can be zero, fallback to some value
			xdpi = 160.0f;
			ydpi = 160.0f;
		}
		window->setSize(core::WindowSize {Size2UI {(unsigned int)(bounds.Width * resolutionScale + 0.5f), (unsigned int)(bounds.Height * resolutionScale + 0.5f)}, Vector2F {xdpi, ydpi}});
	}
	static void changeWindowSize(core::Window* window, ::Windows::Graphics::Display::DisplayInformation^ dinfo) {
		float xdpi = dinfo->RawDpiX;
		float ydpi = dinfo->RawDpiY;
		if (xdpi == 0.0f || ydpi == 0.0f) {
			LOGW() << "DPI values are not available";
			//these can be zero, fallback to some value
			xdpi = 160.0f;
			ydpi = 160.0f;
		}
		const auto& size = window->getWindowSize();
		window->setSize(core::WindowSize {size.pixelSize, Vector2F {xdpi, ydpi}});
	}
	static void changeWindowSize(core::Window* window, const ::Windows::Foundation::Rect& bounds) {
		const auto& size = window->getWindowSize();
		window->setSize(
				core::WindowSize { Size2UI { (unsigned int) (bounds.Width * resolutionScale + 0.5f), (unsigned int) (bounds.Height
						* resolutionScale + 0.5f) }, size.dpi });
	}
	static core::Window* createNewWindow(::Windows::UI::Core::CoreWindow^ window, ::Windows::Graphics::Display::DisplayInformation^ dinfo) {
		core::Window* result = new core::Window(window);
		core::Window::addWindow(*result, nullptr);	//TODO non-nullptr args
		changeWindowSize(result, window->Bounds, dinfo);
		return result;
	}
	static void closeWindow(::Windows::UI::Core::CoreWindow^ window) {
		LOGTRACE();
		core::Window* w = getWindowForCoreWindow(window);
		core::Window::removeWindow(*w);
		delete w;
	}
	static void setVisibility(core::Window* window, bool visible) {
		window->setVisibleToUser(visible, getMonotonicTime());
	}
};

// The first method called when the IFrameworkView is being created.
void App::Initialize(::Windows::ApplicationModel::Core::CoreApplicationView^ applicationView) {

	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	QueryPerformanceFrequency(&performancefrequency);

	platform_bridge::initMainThread();
	platform_bridge::updateTime();

	//::rhfw::module::initialize_modules();

	StorageDirectoryDescriptor::InitializePlatformRootDirectory();

	user_app_initialize(0, nullptr);

	applicationView->Activated +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::ApplicationModel::Core::CoreApplicationView^, ::Windows::ApplicationModel::Activation::IActivatedEventArgs^>(this, &App::OnActivated);

	::Windows::ApplicationModel::Core::CoreApplication::Suspending +=
		ref new ::Windows::Foundation::EventHandler<::Windows::ApplicationModel::SuspendingEventArgs^>(this, &App::OnSuspending);

	::Windows::ApplicationModel::Core::CoreApplication::Resuming +=
		ref new ::Windows::Foundation::EventHandler<::Platform::Object^>(this, &App::OnResuming);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(::Windows::UI::Core::CoreWindow^ window) {
	window->SizeChanged +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->PointerPressed +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerPressed);
	window->PointerMoved +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerMoved);
	window->PointerReleased +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerReleased);
	window->PointerWheelChanged +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerWheelChanged);

	window->KeyDown +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow ^, ::Windows::UI::Core::KeyEventArgs ^>(this, &App::OnKeyDown);
	window->KeyUp +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow ^, ::Windows::UI::Core::KeyEventArgs ^>(this, &App::OnKeyUp);
	window->CharacterReceived +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow ^, ::Windows::UI::Core::CharacterReceivedEventArgs ^>(this, &App::OnCharacterReceived);

	window->Dispatcher->AcceleratorKeyActivated +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreDispatcher ^, ::Windows::UI::Core::AcceleratorKeyEventArgs ^>(this, &App::OnAcceleratorKeyActivated);

	window->Closed +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::UI::Core::CoreWindow^, ::Windows::UI::Core::CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	auto currentDisplayInformation = ::Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::Graphics::Display::DisplayInformation^, Object^>(this, &App::OnDpiChanged);
	currentDisplayInformation->OrientationChanged +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::Graphics::Display::DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	::Windows::Graphics::Display::DisplayInformation::DisplayContentsInvalidated +=
		ref new ::Windows::Foundation::TypedEventHandler<::Windows::Graphics::Display::DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	::Windows::UI::Core::SystemNavigationManager::GetForCurrentView()->BackRequested += ref new ::Windows::Foundation::EventHandler<::Windows::UI::Core::BackRequestedEventArgs ^>(this, &App::BackRequested);

	resolutionScale = getResolutionScale(currentDisplayInformation);
	platform_bridge::createNewWindow(window, currentDisplayInformation);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(::Platform::String^ entryPoint) {
	::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->PreferredLaunchWindowingMode = ::Windows::UI::ViewManagement::ApplicationViewWindowingMode::FullScreen;
	::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->FullScreenSystemOverlayMode = ::Windows::UI::ViewManagement::FullScreenSystemOverlayMode::Minimal;
	::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->SuppressSystemOverlays = true;
	bool res = ::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TryEnterFullScreenMode();

	//auto window = ::Windows::UI::Core::CoreWindow::GetForCurrentThread();
	//window->SetPointerCapture();
	//CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
}

// This method is called after the window becomes active.
void App::Run() {
	auto dispatcher = ::Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

	while (!core::Window::getWindows().isEmpty()) {
		platform_bridge::updateTime();
		const bool didDraw = platform_bridge::executeDrawing();

		const bool waitInfinite = !core::GlobalMonotonicTimeListener::hasListeners() && !didDraw;

		dispatcher->ProcessEvents(
				waitInfinite ?
						::Windows::UI::Core::CoreProcessEventsOption::ProcessOneAndAllPending :
						::Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize() {
}

// Application lifecycle event handlers.

void App::BackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ e) {
	bool handled;

	auto* window = platform_bridge::getWindowForCoreWindow(::Windows::UI::Core::CoreWindow::GetForCurrentThread());

	KeyMessage::ExtraData extra;
	extra.keycode = KeyCode::KEY_BACK;
	extra.modifiers = KeyModifiers::NO_FLAG;
	handled = KeyMessage::postMessage(window, InputDevice::KEYBOARD, KeyAction::DOWN, extra);

	extra.keycode = KeyCode::KEY_BACK;
	extra.modifiers = KeyModifiers::NO_FLAG;
	handled = handled | KeyMessage::postMessage(window, InputDevice::KEYBOARD, KeyAction::UP, extra);

	e->Handled = handled;
}

void App::OnActivated(::Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, ::Windows::ApplicationModel::Activation::IActivatedEventArgs^ args) {
	// Run() won't start until the CoreWindow is activated.
	applicationView->CoreWindow->Activate();
	//::Windows::UI::Core::CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(::Platform::Object^ sender, ::Windows::ApplicationModel::SuspendingEventArgs^ args) {
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	//auto deferral = args->SuspendingOperation->GetDeferral();

	//::concurrency::create_task([this, deferral]() {
	for (auto&& l : rhfw::windowsstore::ApplicationStateListener::foreach()) {
		l.applicationSuspending();
	}
	//deferral->Complete();
//});
}
void App::OnResuming(::Platform::Object^ sender, ::Platform::Object^ args) {
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	for (auto&& l : rhfw::windowsstore::ApplicationStateListener::foreach()) {
		l.applicationResuming();
	}
}

// Window event handlers.
void App::OnWindowSizeChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::WindowSizeChangedEventArgs^ args) {
	platform_bridge::changeWindowSize(platform_bridge::getWindowForCoreWindow(sender), sender->Bounds);
}
void App::OnVisibilityChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::VisibilityChangedEventArgs^ args) {
	platform_bridge::setVisibility(platform_bridge::getWindowForCoreWindow(sender), args->Visible);
}
void App::OnWindowClosed(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::CoreWindowEventArgs^ args) {
	platform_bridge::closeWindow(sender);
}
void App::OnDpiChanged(::Windows::Graphics::Display::DisplayInformation^ sender, Object^ args) {
	resolutionScale = getResolutionScale(sender);
	platform_bridge::changeWindowSize(platform_bridge::getWindowForCoreWindow(::Windows::UI::Core::CoreWindow::GetForCurrentThread()), sender);
}

void App::OnOrientationChanged(::Windows::Graphics::Display::DisplayInformation^ sender, Object^ args) {
}
void App::OnDisplayContentsInvalidated(::Windows::Graphics::Display::DisplayInformation^ sender, Object^ args) {
}

static InputDevice convertPointerType(Windows::Devices::Input::PointerDeviceType type) {
	switch (type) {
		case Windows::Devices::Input::PointerDeviceType::Touch:
			return InputDevice::FINGER;
		case Windows::Devices::Input::PointerDeviceType::Pen:
			return InputDevice::STYLUS;
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			return InputDevice::MOUSE;
		default:
			return InputDevice::UNKNOWN;
	}
}
//If device is mouse, the id should be 0
//PointerId is incrementing with each touch, but not with mouse
#define TO_TOUCH_ID(point) ((point)->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse ? 1 : ((point)->PointerId % (TouchEvent::MAX_POINTER_COUNT - 1)) + 1)

void App::OnPointerPressed(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args) {
	//if (args->Handled)
	//	return;
	//auto inputpane = Windows::UI::ViewManagement::InputPane::GetForCurrentView();
	//bool res = inputpane->TryShow();

	//LOGV("pointer id: %d", args->CurrentPoint->PointerId);
	//touch input id is increasing always

	args->Handled = true;

	TouchMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender),
							  convertPointerType(args->CurrentPoint->PointerDevice->PointerDeviceType),
							  TO_TOUCH_ID(args->CurrentPoint),
							  TouchAction::DOWN,
							  args->CurrentPoint->Position.X * resolutionScale,
							  args->CurrentPoint->Position.Y * resolutionScale,
							  getMonotonicTime());
	//LOGTRACE() << TO_TOUCH_ID(args->CurrentPoint);
}
void App::OnPointerMoved(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args) {
	//if (args->Handled)
	//	return;
	args->Handled = true;

	//LOGV("pointer id: %d, pos: %f, %f", args->CurrentPoint->PointerId, args->CurrentPoint->Position.X * resolutionScale, args->CurrentPoint->Position.Y * resolutionScale);

	if (args->CurrentPoint->IsInContact) {
		TouchMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender),
								  convertPointerType(args->CurrentPoint->PointerDevice->PointerDeviceType),
								  TO_TOUCH_ID(args->CurrentPoint),
								  TouchAction::MOVED_SINGLE,
								  args->CurrentPoint->Position.X * resolutionScale,
								  args->CurrentPoint->Position.Y * resolutionScale,
								  getMonotonicTime());
		//LOGTRACE() << TO_TOUCH_ID(args->CurrentPoint);
	}
}
void App::OnPointerReleased(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args) {
	//if (args->Handled)
	//	return;
	args->Handled = true;
	TouchMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender),
							  convertPointerType(args->CurrentPoint->PointerDevice->PointerDeviceType),
							  TO_TOUCH_ID(args->CurrentPoint),
							  TouchAction::UP,
							  args->CurrentPoint->Position.X * resolutionScale,
							  args->CurrentPoint->Position.Y * resolutionScale,
							  getMonotonicTime());
	//LOGTRACE() << TO_TOUCH_ID(args->CurrentPoint);
}
void App::OnPointerWheelChanged(::Windows::UI::Core::CoreWindow^ sender, ::Windows::UI::Core::PointerEventArgs^ args) {
	LOGTRACE() << args->CurrentPoint->Properties->MouseWheelDelta;
	args->Handled = true;
	TouchEvent::ExtraData extra;
	extra.wheel = args->CurrentPoint->Properties->MouseWheelDelta;
	extra.wheelMax = WHEEL_DELTA;
	TouchMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender),
							  convertPointerType(args->CurrentPoint->PointerDevice->PointerDeviceType),
							  TO_TOUCH_ID(args->CurrentPoint),
							  TouchAction::WHEEL,
							  args->CurrentPoint->Position.X * resolutionScale,
							  args->CurrentPoint->Position.Y * resolutionScale,
							  getMonotonicTime(), extra);
}

//https://msdn.microsoft.com/EN-US/library/windows/apps/windows.system.virtualkey.aspx
static const KeyCode KEYCODE_MAP[256] = {
/*   	0	: None           */KeyCode::KEY_UNKNOWN,
/*   	1	: LeftButton     */KeyCode::KEY_UNKNOWN,
/*   	2	: RightButton    */KeyCode::KEY_UNKNOWN,
/*   	3	: Cancel         */KeyCode::KEY_UNKNOWN,
/*   	4	: MiddleButton   */KeyCode::KEY_UNKNOWN,
/*   	5	: XButton1       */KeyCode::KEY_UNKNOWN,
/*   	6	: XButton2       */KeyCode::KEY_UNKNOWN,
/*   	7	: -              */KeyCode::KEY_UNKNOWN,
/*   	8	: Back           */KeyCode::KEY_BACKSPACE,
/*   	9	: Tab            */KeyCode::KEY_TAB,
/*   	10	: -              */KeyCode::KEY_UNKNOWN,
/*   	11	: -              */KeyCode::KEY_UNKNOWN,
/*   	12	: Clear          */KeyCode::KEY_CLEAR,
/*   	13	: Enter          */KeyCode::KEY_ENTER,
/*   	14	: -              */KeyCode::KEY_UNKNOWN,
/*   	15	: -              */KeyCode::KEY_UNKNOWN,
/*   	16	: Shift          */KeyCode::KEY_SHIFT,
/*   	17	: Control        */KeyCode::KEY_CTRL,
/*   	18	: Menu           */KeyCode::KEY_ALT,
/*   	19	: Pause          */KeyCode::KEY_PAUSEBREAK,
/*   	20	: CapitalLock    */KeyCode::KEY_CAPSLOCK,
/*   	21	: Kana           */KeyCode::KEY_UNKNOWN,
/*   	22	: -              */KeyCode::KEY_UNKNOWN,
/*   	23	: Junja          */KeyCode::KEY_UNKNOWN,
/*   	24	: Final          */KeyCode::KEY_UNKNOWN,
/*   	25	: Hanja          */KeyCode::KEY_UNKNOWN,
/*   	26	: -              */KeyCode::KEY_UNKNOWN,
/*   	27	: Escape         */KeyCode::KEY_ESC,
/*   	28	: Convert        */KeyCode::KEY_UNKNOWN,
/*   	29	: NonConvert     */KeyCode::KEY_UNKNOWN,
/*   	30	: Accept         */KeyCode::KEY_UNKNOWN,
/*   	31	: ModeChange     */KeyCode::KEY_UNKNOWN,
/*   	32	: Space          */KeyCode::KEY_SPACE,
/*   	33	: PageUp         */KeyCode::KEY_PAGE_UP,
/*   	34	: PageDown       */KeyCode::KEY_PAGE_DOWN,
/*   	35	: End            */KeyCode::KEY_END,
/*   	36	: Home           */KeyCode::KEY_HOME,
/*   	37	: Left           */KeyCode::KEY_DIR_LEFT,
/*   	38	: Up             */KeyCode::KEY_DIR_UP,
/*   	39	: Right          */KeyCode::KEY_DIR_RIGHT,
/*   	40	: Down           */KeyCode::KEY_DIR_DOWN,
/*   	41	: Select         */KeyCode::KEY_UNKNOWN,
/*   	42	: Print          */KeyCode::KEY_UNKNOWN,
/*   	43	: Executer       */KeyCode::KEY_UNKNOWN,
/*   	44	: Snapshot       */KeyCode::KEY_UNKNOWN,
/*   	45	: Insert         */KeyCode::KEY_INSERT,
/*   	46	: Delete         */KeyCode::KEY_DELETE,
/*   	47	: Help           */KeyCode::KEY_UNKNOWN,
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
/*   	91	: LeftWindows    */KeyCode::KEY_UNKNOWN,
/*   	92	: RightWindows   */KeyCode::KEY_UNKNOWN,
/*   	93	: Application    */KeyCode::KEY_UNKNOWN,
/*   	94	: -              */KeyCode::KEY_UNKNOWN,
/*   	95	: Sleep          */KeyCode::KEY_UNKNOWN,
/*   	96	: NumberPad0     */KeyCode::KEY_NUM_0,
/*   	97	: NumberPad1     */KeyCode::KEY_NUM_1,
/*   	98	: NumberPad2     */KeyCode::KEY_NUM_2,
/*   	99	: NumberPad3     */KeyCode::KEY_NUM_3,
/*   	100	: NumberPad4     */KeyCode::KEY_NUM_4,
/*   	101	: NumberPad5     */KeyCode::KEY_NUM_5,
/*   	102	: NumberPad6     */KeyCode::KEY_NUM_6,
/*   	103	: NumberPad7     */KeyCode::KEY_NUM_7,
/*   	104	: NumberPad8     */KeyCode::KEY_NUM_8,
/*   	105	: NumberPad9     */KeyCode::KEY_NUM_9,
/*   	106	: Multiply       */KeyCode::KEY_NUM_MULT,
/*   	107	: Add            */KeyCode::KEY_NUM_ADD,
/*   	108	: Separator      */KeyCode::KEY_UNKNOWN,
/*   	109	: Subtract       */KeyCode::KEY_NUM_SUBTRACT,
/*   	110	: Decimal        */KeyCode::KEY_NUM_DOT,
/*   	111	: Divide         */KeyCode::KEY_NUM_DIV,
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
/*   	136	: NavigationView */KeyCode::KEY_UNKNOWN,
/*   	137	: NavigationMenu */KeyCode::KEY_UNKNOWN,
/*   	138	: NavigationUp   */KeyCode::KEY_UNKNOWN,
/*   	139	: NavigationDown */KeyCode::KEY_UNKNOWN,
/*   	140	: NavigationLeft */KeyCode::KEY_UNKNOWN,
/*   	141	: NavigationRight*/KeyCode::KEY_UNKNOWN,
/*   	142	: NavigationAccept*/KeyCode::KEY_UNKNOWN,
/*   	143	: NavigationCancel*/KeyCode::KEY_UNKNOWN,
/*   	144	: NumberKeyLock  */KeyCode::KEY_NUMLOCK,
/*   	145	: Scroll         */KeyCode::KEY_SCROLLLOCK,
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
/*   	160	: LeftShift      */KeyCode::KEY_LEFT_SHIFT,
/*   	161	: RightShift     */KeyCode::KEY_RIGHT_SHIFT,
/*   	162	: LeftControl    */KeyCode::KEY_LEFT_CTRL,
/*   	163	: RightControl   */KeyCode::KEY_RIGHT_CTRL,
/*   	164	: LeftMenu       */KeyCode::KEY_LEFT_ALT,
/*   	165	: RightMenu      */KeyCode::KEY_RIGHT_ALT,
/*   	166	: GoBack         */KeyCode::KEY_UNKNOWN,
/*   	167	: GoForward      */KeyCode::KEY_UNKNOWN,
/*   	168	: Refresh        */KeyCode::KEY_UNKNOWN,
/*   	169	: Stop           */KeyCode::KEY_UNKNOWN,
/*   	170	: Search         */KeyCode::KEY_UNKNOWN,
/*   	171	: Favorites      */KeyCode::KEY_UNKNOWN,
/*   	172	: GoHome         */KeyCode::KEY_UNKNOWN,
/*   	173	: -              */KeyCode::KEY_UNKNOWN,
/*   	174	: -              */KeyCode::KEY_UNKNOWN,
/*   	175	: -              */KeyCode::KEY_UNKNOWN,
/*   	176	: -              */KeyCode::KEY_UNKNOWN,
/*   	177	: -              */KeyCode::KEY_UNKNOWN,
/*   	178	: -              */KeyCode::KEY_UNKNOWN,
/*   	179	: -              */KeyCode::KEY_UNKNOWN,
/*   	180	: -              */KeyCode::KEY_UNKNOWN,
/*   	181	: -              */KeyCode::KEY_UNKNOWN,
/*   	182	: -              */KeyCode::KEY_UNKNOWN,
/*   	183	: -              */KeyCode::KEY_UNKNOWN,
/*   	184	: -              */KeyCode::KEY_UNKNOWN,
/*   	185	: -              */KeyCode::KEY_UNKNOWN,
/*   	186	: -              */KeyCode::KEY_UNKNOWN,
/*   	187	: -              */KeyCode::KEY_UNKNOWN,
/*   	188	: -              */KeyCode::KEY_UNKNOWN,
/*   	189	: -              */KeyCode::KEY_UNKNOWN,
/*   	190	: -              */KeyCode::KEY_UNKNOWN,
/*   	191	: -              */KeyCode::KEY_UNKNOWN,
/*   	192	: -              */KeyCode::KEY_UNKNOWN,
/*   	193	: -              */KeyCode::KEY_UNKNOWN,
/*   	194	: -              */KeyCode::KEY_UNKNOWN,
/*   	195	: GamepadA       */KeyCode::KEY_UNKNOWN,
/*   	196	: GamepadB       */KeyCode::KEY_UNKNOWN,
/*   	197	: GamepadX       */KeyCode::KEY_UNKNOWN,
/*   	198	: GamepadY       */KeyCode::KEY_UNKNOWN,
/*   	199	: GpRightShoulder*/KeyCode::KEY_UNKNOWN,
/*   	200	: GpLeftShoulder */KeyCode::KEY_UNKNOWN,
/*   	201	: GpLeftTrigger  */KeyCode::KEY_UNKNOWN,
/*   	202	: GpRightTrigger */KeyCode::KEY_UNKNOWN,
/*   	203	: GpDPadUp       */KeyCode::KEY_UNKNOWN,
/*   	204	: GpDPadDown     */KeyCode::KEY_UNKNOWN,
/*   	205	: GpDPadLeft     */KeyCode::KEY_UNKNOWN,
/*   	206	: GpDPadRight    */KeyCode::KEY_UNKNOWN,
/*   	207	: GamepadMenu    */KeyCode::KEY_UNKNOWN,
/*   	208	: GamepadView    */KeyCode::KEY_UNKNOWN,
/*   	209	: GpLeftThumbBtn */KeyCode::KEY_UNKNOWN,
/*   	210	: GpRightThumbBtn*/KeyCode::KEY_UNKNOWN,
/*   	211	: GpLeftThumbUp  */KeyCode::KEY_UNKNOWN,
/*   	212	: GpLeftThumbDown*/KeyCode::KEY_UNKNOWN,
/*   	213	: GpLeftThumbRight*/KeyCode::KEY_UNKNOWN,
/*   	214	: GpLeftThumbLeft*/KeyCode::KEY_UNKNOWN,
/*   	215	: GpRightThumbUp */KeyCode::KEY_UNKNOWN,
/*   	216	: GpRightThumbDown*/KeyCode::KEY_UNKNOWN,
/*   	217	: GpRightThumbRight*/KeyCode::KEY_UNKNOWN,
/*   	218	: GpRightThumbLeft*/KeyCode::KEY_UNKNOWN,
/*   	219	: -              */KeyCode::KEY_UNKNOWN,
/*   	220	: -              */KeyCode::KEY_UNKNOWN,
/*   	221	: -              */KeyCode::KEY_UNKNOWN,
/*   	222	: -              */KeyCode::KEY_UNKNOWN,
/*   	223	: -              */KeyCode::KEY_UNKNOWN,
/*   	224	: -              */KeyCode::KEY_UNKNOWN,
/*   	225	: -              */KeyCode::KEY_UNKNOWN,
/*   	226	: -              */KeyCode::KEY_UNKNOWN,
/*   	227	: -              */KeyCode::KEY_UNKNOWN,
/*   	228	: -              */KeyCode::KEY_UNKNOWN,
/*   	229	: -              */KeyCode::KEY_UNKNOWN,
/*   	230	: -              */KeyCode::KEY_UNKNOWN,
/*   	231	: -              */KeyCode::KEY_UNKNOWN,
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
/*   	246	: -              */KeyCode::KEY_UNKNOWN,
/*   	247	: -              */KeyCode::KEY_UNKNOWN,
/*   	248	: -              */KeyCode::KEY_UNKNOWN,
/*   	249	: -              */KeyCode::KEY_UNKNOWN,
/*   	250	: -              */KeyCode::KEY_UNKNOWN,
/*   	251	: -              */KeyCode::KEY_UNKNOWN,
/*   	252	: -              */KeyCode::KEY_UNKNOWN,
/*   	253	: -              */KeyCode::KEY_UNKNOWN,
/*   	254	: -              */KeyCode::KEY_UNKNOWN,
/*   	255	: -              */KeyCode::KEY_UNKNOWN, };

static rhfw::KeyModifierTracker modifier_states;

void App::OnKeyDown(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::KeyEventArgs ^args) {
	args->Handled = true;

	KeyMessage::ExtraData extra;
	extra.keycode = KEYCODE_MAP[(unsigned char)args->VirtualKey];	//0-255
	modifier_states.down(extra.keycode);
	extra.modifiers = modifier_states;
	//RepeatCount is always 1.... use WasKeyDown
	extra.repeat = args->KeyStatus.WasKeyDown;
	LOGTRACE() << "OnKeyDown: " << (unsigned int)args->VirtualKey << ": " << extra.keycode;
	KeyMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender), InputDevice::KEYBOARD, KeyAction::DOWN, extra);
	//LOGV("OnKeyDown: virtualkey: %d, handled: %d, keyenum: %s, %llu",
	//	 (int)args->VirtualKey, (int)args->Handled, TOSTRING(extra.keycode), GetTickCount64());
}
void App::OnKeyUp(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::KeyEventArgs ^args) {
	args->Handled = true;

	KeyMessage::ExtraData extra;
	extra.keycode = KEYCODE_MAP[(unsigned char)args->VirtualKey];	//0-255
	modifier_states.up(extra.keycode);
	extra.modifiers = modifier_states;
	LOGTRACE() << "OnKeyUp: " << (unsigned int)args->VirtualKey << ": " << extra.keycode;
	KeyMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender), InputDevice::KEYBOARD, KeyAction::UP, extra);

	//LOGV("OnKeyUp: virtualkey: %d, handled: %d, keyenum: %s, %llu",
	//	 (int)args->VirtualKey, (int)args->Handled, TOSTRING(extra.keycode), GetTickCount64());
}
void App::OnCharacterReceived(::Windows::UI::Core::CoreWindow ^sender, ::Windows::UI::Core::CharacterReceivedEventArgs ^args) {
	//there is no combining accent, combined character result is delayed
	args->Handled = true;
	//TODO nem static, hanem state valtozoba
	static unsigned int lastHighSurrogate = 0;
	unsigned int keycode = args->KeyCode;
	if(IS_HIGH_SURROGATE(keycode)) {
		//current is high surrogate, wait for low in next event
		lastHighSurrogate = keycode;
	} else {
		if (IS_SURROGATE_PAIR(lastHighSurrogate, keycode)) {
			keycode = 0x10000 | ((lastHighSurrogate & 0x3FF) << 10) | (keycode & 0x3FF);
			lastHighSurrogate = 0;
		}
		//send unicode
		KeyMessage::ExtraData extra;
		extra.repeatUnicode = keycode;
		extra.repeatUnicodeCount = 1;
		//do not deliver this yet, not supported by application
		KeyMessage::postMessage(platform_bridge::getWindowForCoreWindow(sender), InputDevice::KEYBOARD, KeyAction::UNICODE_REPEAT, extra);
	}
}

void App::OnAcceleratorKeyActivated(::Windows::UI::Core::CoreDispatcher^ sender, ::Windows::UI::Core::AcceleratorKeyEventArgs^ args) {
	switch (args->VirtualKey) {
		case ::Windows::System::VirtualKey::RightMenu:
		case ::Windows::System::VirtualKey::LeftMenu:
		case ::Windows::System::VirtualKey::Menu: {
			KeyMessage::ExtraData extra;
			extra.keycode = KEYCODE_MAP[(unsigned char)args->VirtualKey];	//0-255
			modifier_states.down(extra.keycode);
			extra.modifiers = modifier_states;
			//RepeatCount is always 1.... use WasKeyDown
			extra.repeat = args->KeyStatus.WasKeyDown;
			if(args->EventType == ::Windows::UI::Core::CoreAcceleratorKeyEventType::KeyDown || args->EventType == ::Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown) {
				KeyMessage::postMessage(platform_bridge::getWindowForCoreWindow(::Windows::UI::Core::CoreWindow::GetForCurrentThread()), InputDevice::KEYBOARD,
						KeyAction::DOWN, extra);
			} else if(args->EventType == ::Windows::UI::Core::CoreAcceleratorKeyEventType::KeyUp || args->EventType == ::Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp) {
				KeyMessage::postMessage(platform_bridge::getWindowForCoreWindow(::Windows::UI::Core::CoreWindow::GetForCurrentThread()), InputDevice::KEYBOARD,
						KeyAction::UP, extra);
			}
			break;
		}
		default: {
			break;
		}
	}
}

/*
 void applyPlatformConfigurations(ResourceConfiguration& config) {

 }
 */
}

// The main function is only used to initialize our IFrameworkView class.
[::Platform::MTAThread]
int main(::Platform::Array<::Platform::String^>^) {
	auto direct3DApplicationSource = ref new rhfw::Direct3DApplicationSource();
	::Windows::ApplicationModel::Core::CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
