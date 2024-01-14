/*  KingStation - A frontend for libks.
 *  Copyright (C) 2018 - Krzysztof Haładyn
 *
 *  KingStation is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  KingStation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with KingStation.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ppltasks.h>
#include <collection.h>
#include <windows.devices.enumeration.h>

#include <encodings/utf.h>
#include <string/stdstring.h>
#include <lists/string_list.h>
#include <queues/task_queue.h>
#include <ks_timers.h>

#include "configuration.h"
#include "paths.h"

#include "uwp_main.h"
#include "../KingStation.h"
#include "../frontend/frontend.h"
#include "../input/input_keymaps.h"
#include "../verbosity.h"
#include "uwp_func.h"
#include "uwp_async.h"

using namespace KingStationUWP;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Devices::Input;
using namespace Windows::System;
using namespace Windows::System::Profile;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Devices::Enumeration;

char uwp_dir_install[PATH_MAX_LENGTH] = { 0 };
char uwp_dir_data[PATH_MAX_LENGTH]    = { 0 };
char uwp_device_family[128]           = { 0 };
char win32_cpu_model_name[128]        = { 0 };

// Some keys are unavailable in the VirtualKey enum (wtf) but the old-style constants work
const struct kingsn_key_map kingsn_key_map_uwp[] = {
   { (unsigned int)VirtualKey::Back, KSK_BACKSPACE },
   { (unsigned int)VirtualKey::Tab, KSK_TAB },
   { (unsigned int)VirtualKey::Clear, KSK_CLEAR },
   { (unsigned int)VirtualKey::Enter, KSK_RETURN },
   { (unsigned int)VirtualKey::Pause, KSK_PAUSE },
   { (unsigned int)VirtualKey::Escape, KSK_ESCAPE },
   { (unsigned int)VirtualKey::ModeChange, KSK_MODE },
   { (unsigned int)VirtualKey::Space, KSK_SPACE },
   { (unsigned int)VirtualKey::PageUp, KSK_PAGEUP },
   { (unsigned int)VirtualKey::PageDown, KSK_PAGEDOWN },
   { (unsigned int)VirtualKey::End, KSK_END },
   { (unsigned int)VirtualKey::Home, KSK_HOME },
   { (unsigned int)VirtualKey::Left, KSK_LEFT },
   { (unsigned int)VirtualKey::Up, KSK_UP },
   { (unsigned int)VirtualKey::Right, KSK_RIGHT },
   { (unsigned int)VirtualKey::Down, KSK_DOWN },
   { (unsigned int)VirtualKey::Print, KSK_PRINT },
   { (unsigned int)VirtualKey::Insert, KSK_INSERT },
   { (unsigned int)VirtualKey::Delete, KSK_DELETE },
   { (unsigned int)VirtualKey::Help, KSK_HELP },
   { (unsigned int)VirtualKey::Number0, KSK_0 },
   { (unsigned int)VirtualKey::Number1, KSK_1 },
   { (unsigned int)VirtualKey::Number2, KSK_2 },
   { (unsigned int)VirtualKey::Number3, KSK_3 },
   { (unsigned int)VirtualKey::Number4, KSK_4 },
   { (unsigned int)VirtualKey::Number5, KSK_5 },
   { (unsigned int)VirtualKey::Number6, KSK_6 },
   { (unsigned int)VirtualKey::Number7, KSK_7 },
   { (unsigned int)VirtualKey::Number8, KSK_8 },
   { (unsigned int)VirtualKey::Number9, KSK_9 },
   { (unsigned int)VirtualKey::A, KSK_a },
   { (unsigned int)VirtualKey::B, KSK_b },
   { (unsigned int)VirtualKey::C, KSK_c },
   { (unsigned int)VirtualKey::D, KSK_d },
   { (unsigned int)VirtualKey::E, KSK_e },
   { (unsigned int)VirtualKey::F, KSK_f },
   { (unsigned int)VirtualKey::G, KSK_g },
   { (unsigned int)VirtualKey::H, KSK_h },
   { (unsigned int)VirtualKey::I, KSK_i },
   { (unsigned int)VirtualKey::J, KSK_j },
   { (unsigned int)VirtualKey::K, KSK_k },
   { (unsigned int)VirtualKey::L, KSK_l },
   { (unsigned int)VirtualKey::M, KSK_m },
   { (unsigned int)VirtualKey::N, KSK_n },
   { (unsigned int)VirtualKey::O, KSK_o },
   { (unsigned int)VirtualKey::P, KSK_p },
   { (unsigned int)VirtualKey::Q, KSK_q },
   { (unsigned int)VirtualKey::R, KSK_r },
   { (unsigned int)VirtualKey::S, KSK_s },
   { (unsigned int)VirtualKey::T, KSK_t },
   { (unsigned int)VirtualKey::U, KSK_u },
   { (unsigned int)VirtualKey::V, KSK_v },
   { (unsigned int)VirtualKey::W, KSK_w },
   { (unsigned int)VirtualKey::X, KSK_x },
   { (unsigned int)VirtualKey::Y, KSK_y },
   { (unsigned int)VirtualKey::Z, KSK_z },
   { (unsigned int)VirtualKey::LeftWindows, KSK_LSUPER },
   { (unsigned int)VirtualKey::RightWindows, KSK_RSUPER },
   { (unsigned int)VirtualKey::Application, KSK_MENU },
   { (unsigned int)VirtualKey::NumberPad0, KSK_KP0 },
   { (unsigned int)VirtualKey::NumberPad1, KSK_KP1 },
   { (unsigned int)VirtualKey::NumberPad2, KSK_KP2 },
   { (unsigned int)VirtualKey::NumberPad3, KSK_KP3 },
   { (unsigned int)VirtualKey::NumberPad4, KSK_KP4 },
   { (unsigned int)VirtualKey::NumberPad5, KSK_KP5 },
   { (unsigned int)VirtualKey::NumberPad6, KSK_KP6 },
   { (unsigned int)VirtualKey::NumberPad7, KSK_KP7 },
   { (unsigned int)VirtualKey::NumberPad8, KSK_KP8 },
   { (unsigned int)VirtualKey::NumberPad9, KSK_KP9 },
   { (unsigned int)VirtualKey::Multiply, KSK_KP_MULTIPLY },
   { (unsigned int)VirtualKey::Add, KSK_KP_PLUS },
   { (unsigned int)VirtualKey::Subtract, KSK_KP_MINUS },
   { (unsigned int)VirtualKey::Decimal, KSK_KP_PERIOD },
   { (unsigned int)VirtualKey::Divide, KSK_KP_DIVIDE },
   { (unsigned int)VirtualKey::F1, KSK_F1 },
   { (unsigned int)VirtualKey::F2, KSK_F2 },
   { (unsigned int)VirtualKey::F3, KSK_F3 },
   { (unsigned int)VirtualKey::F4, KSK_F4 },
   { (unsigned int)VirtualKey::F5, KSK_F5 },
   { (unsigned int)VirtualKey::F6, KSK_F6 },
   { (unsigned int)VirtualKey::F7, KSK_F7 },
   { (unsigned int)VirtualKey::F8, KSK_F8 },
   { (unsigned int)VirtualKey::F9, KSK_F9 },
   { (unsigned int)VirtualKey::F10, KSK_F10 },
   { (unsigned int)VirtualKey::F11, KSK_F11 },
   { (unsigned int)VirtualKey::F12, KSK_F12 },
   { (unsigned int)VirtualKey::F13, KSK_F13 },
   { (unsigned int)VirtualKey::F14, KSK_F14 },
   { (unsigned int)VirtualKey::F15, KSK_F15 },
   { (unsigned int)VirtualKey::NumberKeyLock, KSK_NUMLOCK },
   { (unsigned int)VirtualKey::Scroll, KSK_SCROLLOCK },
   { (unsigned int)VirtualKey::LeftShift, KSK_LSHIFT },
   { (unsigned int)VirtualKey::RightShift, KSK_RSHIFT },
   { (unsigned int)VirtualKey::LeftControl, KSK_LCTRL },
   { (unsigned int)VirtualKey::RightControl, KSK_RCTRL },
   { (unsigned int)VirtualKey::LeftMenu, KSK_LALT },
   { (unsigned int)VirtualKey::RightMenu, KSK_RALT },
   { VK_RETURN, KSK_KP_ENTER },
   { (unsigned int)VirtualKey::CapitalLock, KSK_CAPSLOCK },
   { VK_OEM_1, KSK_SEMICOLON },
   { VK_OEM_PLUS, KSK_EQUALS },
   { VK_OEM_COMMA, KSK_COMMA },
   { VK_OEM_MINUS, KSK_MINUS },
   { VK_OEM_PERIOD, KSK_PERIOD },
   { VK_OEM_2, KSK_SLASH },
   { VK_OEM_3, KSK_BACKQUOTE },
   { VK_OEM_4, KSK_LEFTBRACKET },
   { VK_OEM_5, KSK_BACKSLASH },
   { VK_OEM_6, KSK_RIGHTBRACKET },
   { VK_OEM_7, KSK_QUOTE },
   { 0, KSK_UNKNOWN }
};

#define MAX_TOUCH 16
struct input_pointer
{
	int id;
	short x;
   short y;
	short full_x;
   short full_y;
	bool isInContact;
};

struct uwp_input_state_t
{
   struct input_pointer touch[MAX_TOUCH]; /* int alignment */
   unsigned touch_count;
   short mouse_screen_x;
   short mouse_screen_y;
   short mouse_rel_x;
   short mouse_rel_y;
   short mouse_wheel_left;
   short mouse_wheel_up;
   bool mouse_left;
   bool mouse_right;
   bool mouse_middle;
   bool mouse_button4;
   bool mouse_button5;
};

struct uwp_input_state_t uwp_current_input, uwp_next_input;

// Taken from DirectX UWP samples - on Xbox, everything is scaled 200% so getting the DPI calculation correct is crucial
static inline float ConvertDipsToPixels(float dips, float dpi)
{
	static const float dipsPerInch = 96.0f;
	return floorf(dips * dpi / dipsPerInch + 0.5f);
}

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	Platform::String^ install_dir = Windows::ApplicationModel::Package::Current->InstalledLocation->Path + L"\\";
	wcstombs(uwp_dir_install, install_dir->Data(), sizeof(uwp_dir_install));
	Platform::String^ data_dir = Windows::Storage::ApplicationData::Current->LocalFolder->Path + L"\\";
	wcstombs(uwp_dir_data, data_dir->Data(), sizeof(uwp_dir_data));

	wcstombs(uwp_device_family,
         AnalyticsInfo::VersionInfo->DeviceFamily->Data(),
         sizeof(uwp_device_family));

	KINGSN_LOG("Data dir: %ls\n", data_dir->Data());
	KINGSN_LOG("Install dir: %ls\n", install_dir->Data());

	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App^ App::m_instance;

App::App() :
	m_initialized(false),
	m_windowClosed(false),
	m_windowVisible(true),
	m_windowFocused(true),
	m_windowResized(false)
{
	m_instance = this;
}

/* The first method called when the IFrameworkView is being created. */
void App::Initialize(CoreApplicationView^ applicationView)
{
	/* Register event handlers for app lifecycle. This example includes Activated, so that we
	 * can make the CoreWindow active and start rendering on the window. */
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

/* Called when the CoreWindow object is created (or re-created). */
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Activated +=
		ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &App::OnWindowActivated);

	window->Closed +=
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKey);

	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKey);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointer);

	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointer);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointer);

	window->PointerWheelChanged +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointer);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	Windows::UI::Core::SystemNavigationManager::GetForCurrentView()->BackRequested +=
		ref new EventHandler<Windows::UI::Core::BackRequestedEventArgs^>(this, &App::OnBackRequested);
}

/* Initializes scene resources, or loads a previously saved app state. */
void App::Load(Platform::String^ entryPoint)
{
	int ret = kingsn_main(NULL, NULL, NULL);
	if (ret != 0)
	{
		KINGSN_ERR("Init failed\n");
		CoreApplication::Exit();
		return;
	}
	m_initialized = true;

	auto catalog = Windows::ApplicationModel::PackageCatalog::OpenForCurrentPackage();

	catalog->PackageInstalling +=
		ref new TypedEventHandler<PackageCatalog^, PackageInstallingEventArgs^>(this, &App::OnPackageInstalling);
}

/* This method is called after the window becomes active. */
void App::Run()
{
   bool x = false;
	if (!m_initialized)
	{
		KINGSN_WARN("Initialization failed, so not running\n");
		return;
	}


   for (;;)
	{
      int ret;
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		ret = runloop_iterate();

		task_queue_check();

		if (!x)
		{
			/* HACK: I have no idea why is this necessary but 
          * it is required to get proper scaling on Xbox *
			 * Perhaps PreferredLaunchViewSize is broken and 
          * we need to wait until the app starts to call TryResizeView */
			m_windowResized = true;
			x = true;
		}

		if (ret == -1)
			break;
	}
}

/* Required for IFrameworkView.
 * Terminate events do not cause Uninitialize to be called. 
 * It will be called if your IFrameworkView
 * class is torn down while the app is in the foreground. */
void App::Uninitialize()
{
	main_exit(NULL);
}

/* Application lifecycle event handlers. */

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	/* Run() won't start until the CoreWindow is activated. */
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	/* Save app state asynchronously after requesting a deferral. Holding a deferral
	 * indicates that the application is busy performing suspending operations. Be
	 * aware that a deferral may not be held indefinitely. After about five seconds,
	 * the app will be forced to exit.
    */
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
	auto                     app = this;

	create_task([app, deferral]()
	{
		/* TODO: Maybe creating a save state here would be a good idea? */
		settings_t* settings     = config_get_ptr();
      bool config_save_on_exit = settings->bools.config_save_on_exit;

		if (config_save_on_exit)
      {
			if (!path_is_empty(KINGSN_PATH_CONFIG))
			{
				const char* config_path = path_get(KINGSN_PATH_CONFIG);
				bool path_exists        = !string_is_empty(config_path);

            if (path_exists)
            {
               if (config_save_file(config_path))
               {
                  KINGSN_LOG("[config] %s \"%s\".\n",
                        msg_hash_to_str(MSG_SAVED_NEW_CONFIG_TO),
                        config_path);
               }
               else
               {
                  KINGSN_ERR("[config] %s \"%s\".\n",
                     msg_hash_to_str(MSG_FAILED_SAVING_CONFIG_TO),
                     config_path);
               }
            }

			}
		}

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	/* Restore any data or state that was unloaded on suspend. By default, data
	 * and state are persisted when resuming from suspend. Note that this event
	 * does not occur if the app was previously terminated.
    */
}

void App::OnBackRequested(Platform::Object^ sender, Windows::UI::Core::BackRequestedEventArgs^ args)
{
	/* Prevent the B controller button on Xbox One from quitting the app */
	args->Handled = true;
}

/* Window event handlers. */

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_windowResized = true;
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowActivated(CoreWindow^ sender, WindowActivatedEventArgs^ args)
{
	m_windowFocused = args->WindowActivationState != CoreWindowActivationState::Deactivated;
}

void App::OnKey(CoreWindow^ sender, KeyEventArgs^ args)
{
	uint16_t mod = 0;
	if ((sender->GetKeyState(VirtualKey::Shift) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_SHIFT;
	if ((sender->GetKeyState(VirtualKey::Control) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_CTRL;
	if ((sender->GetKeyState(VirtualKey::Menu) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_ALT;
	if ((sender->GetKeyState(VirtualKey::CapitalLock) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_CAPSLOCK;
	if ((sender->GetKeyState(VirtualKey::Scroll) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_SCROLLOCK;
	if ((sender->GetKeyState(VirtualKey::LeftWindows) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked ||
		(sender->GetKeyState(VirtualKey::RightWindows) & CoreVirtualKeyStates::Locked) == CoreVirtualKeyStates::Locked)
		mod |= KSKMOD_META;

	unsigned keycode = input_keymaps_translate_keysym_to_rk((unsigned)args->VirtualKey);

	input_keyboard_event(!args->KeyStatus.IsKeyReleased, keycode, 0, mod, KS_DEVICE_KEYBOARD);
}

void App::OnPointer(CoreWindow^ sender, PointerEventArgs^ args)
{

	float dpi = DisplayInformation::GetForCurrentView()->LogicalDpi;
	
	if (args->CurrentPoint->PointerDevice->PointerDeviceType == PointerDeviceType::Mouse)
	{
		uwp_next_input.mouse_left = args->CurrentPoint->Properties->IsLeftButtonPressed;
		uwp_next_input.mouse_middle = args->CurrentPoint->Properties->IsMiddleButtonPressed;
		uwp_next_input.mouse_right = args->CurrentPoint->Properties->IsRightButtonPressed;
		uwp_next_input.mouse_button4 = args->CurrentPoint->Properties->IsXButton1Pressed;
		uwp_next_input.mouse_button5 = args->CurrentPoint->Properties->IsXButton2Pressed;
		uwp_next_input.mouse_screen_x = ConvertDipsToPixels(args->CurrentPoint->Position.X, dpi);
		uwp_next_input.mouse_screen_y = ConvertDipsToPixels(args->CurrentPoint->Position.Y, dpi);
		uwp_next_input.mouse_rel_x = uwp_next_input.mouse_screen_x - uwp_current_input.mouse_screen_x;
		uwp_next_input.mouse_rel_y = uwp_next_input.mouse_screen_y - uwp_current_input.mouse_screen_y;
		if (args->CurrentPoint->Properties->IsHorizontalMouseWheel)
			uwp_next_input.mouse_wheel_left += args->CurrentPoint->Properties->MouseWheelDelta;
		else
			uwp_next_input.mouse_wheel_up += args->CurrentPoint->Properties->MouseWheelDelta;
	}
	else
	{
		unsigned i, free_index = MAX_TOUCH; bool found = false;
		int id = args->CurrentPoint->PointerId;

		for (i = 0; i < uwp_next_input.touch_count; i++)
		{
			if (!uwp_next_input.touch[i].isInContact && free_index == MAX_TOUCH)
				free_index = i;
			if (uwp_next_input.touch[i].id == id)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			if (free_index >= 0 && free_index < uwp_next_input.touch_count)
				i = free_index;
			else if (uwp_next_input.touch_count + 1 < MAX_TOUCH)
				i = ++uwp_next_input.touch_count;
			else
				return;
		}

		uwp_next_input.touch[i].id = id;

		struct video_viewport vp;

		/* convert from event coordinates to core and screen coordinates */
		vp.x           = 0;
		vp.y           = 0;
		vp.width       = 0;
		vp.height      = 0;
		vp.full_width  = 0;
		vp.full_height = 0;

		video_driver_translate_coord_viewport_wrap(
			&vp,
			ConvertDipsToPixels(args->CurrentPoint->Position.X, dpi),
			ConvertDipsToPixels(args->CurrentPoint->Position.Y, dpi),
			&uwp_next_input.touch[i].x,
			&uwp_next_input.touch[i].y,
			&uwp_next_input.touch[i].full_x,
			&uwp_next_input.touch[i].full_y);

		uwp_next_input.touch[i].isInContact = args->CurrentPoint->IsInContact;
	
	}
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

/* DisplayInformation event handlers. */

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	m_windowResized = true;
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	m_windowResized = true;
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	/* Probably can be ignored? */
}

void App::OnPackageInstalling(PackageCatalog^ sender, PackageInstallingEventArgs^ args)
{
	/* TODO: This doesn't seem to work even though it's exactly the same as in sample app and it works there */
	if (args->IsComplete)
	{
		char msg[512];
		snprintf(msg, sizeof(msg), "Package \"%ls\" installed, a restart may be necessary", args->Package->DisplayName->Data());
		runloop_msg_queue_push(msg, 1, 5 * 60, false, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
	}
}

/* Implement UWP equivalents of various win32_* functions */
extern "C" {

	bool win32_has_focus(void *data)
	{
		return App::GetInstance()->IsWindowFocused();
	}

	bool win32_set_video_mode(void *data, unsigned width, unsigned height, bool fullscreen)
	{
		if (App::GetInstance()->IsInitialized())
		{
			if (fullscreen != ApplicationView::GetForCurrentView()->IsFullScreenMode)
			{
				if (fullscreen)
					ApplicationView::GetForCurrentView()->TryEnterFullScreenMode();
				else
					ApplicationView::GetForCurrentView()->ExitFullScreenMode();
			}
			ApplicationView::GetForCurrentView()->TryResizeView(Size(width, height));
		}
		else
		{
			/* In case the window is not activated yet, TryResizeView will fail and we have to set the initial parameters instead */
			/* Note that these are preserved after restarting the app and used for the UWP splash screen size (!), so they should be set only during init and not changed afterwards */
			ApplicationView::PreferredLaunchViewSize = Size(width, height);
			ApplicationView::PreferredLaunchWindowingMode = fullscreen ? ApplicationViewWindowingMode::FullScreen : ApplicationViewWindowingMode::PreferredLaunchViewSize;
		}

		/* Setting the window size may sometimes fail "because UWP"
		 * (i.e. we are on device with no windows, or Windows sandbox decides the window can't be that small)
		 * so in case resizing fails we just send a resized event back to KingStation with old size
		 * (and report success because otherwise it bails out hard about failing to set video mode)
		 */
		App::GetInstance()->SetWindowResized();
		return true;
	}

	void win32_show_cursor(void *data, bool state)
   {
      CoreWindow::GetForCurrentThread()->PointerCursor = state ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr;
   }


	bool win32_get_metrics(void* data,
		enum display_metric_types type, float* value)
	{
		int pixels_x        = DisplayInformation::GetForCurrentView()->ScreenWidthInRawPixels;
		int pixels_y        = DisplayInformation::GetForCurrentView()->ScreenHeightInRawPixels;
		int raw_dpi_x       = DisplayInformation::GetForCurrentView()->RawDpiX;
		int raw_dpi_y       = DisplayInformation::GetForCurrentView()->RawDpiY;
		int physical_width  = pixels_x / raw_dpi_x;
		int physical_height = pixels_y / raw_dpi_y;

		switch (type)
		{
		case DISPLAY_METRIC_PIXEL_WIDTH:
			*value           = pixels_x;
			return true;
		case DISPLAY_METRIC_PIXEL_HEIGHT:
			*value           = pixels_y;
			return true;
		case DISPLAY_METRIC_MM_WIDTH:
			/* 25.4 mm in an inch. */
			*value           = 254 * physical_width / 10;
			return true;
		case DISPLAY_METRIC_MM_HEIGHT:
			/* 25.4 mm in an inch. */
			*value           = 254 * physical_height / 10;
			return true;
		case DISPLAY_METRIC_DPI:
			*value           = raw_dpi_x;
			return true;
		case DISPLAY_METRIC_NONE:
		default:
			*value           = 0;
			break;
		}
		return false;
	}

	void win32_check_window(void *data,
         bool *quit, bool *resize, unsigned *width, unsigned *height)
	{
		*quit   = App::GetInstance()->IsWindowClosed();
		*resize = App::GetInstance()->CheckWindowResized();
		if (*resize)
		{
			float dpi = DisplayInformation::GetForCurrentView()->LogicalDpi;
			*width    = ConvertDipsToPixels(CoreWindow::GetForCurrentThread()->Bounds.Width, dpi);
			*height   = ConvertDipsToPixels(CoreWindow::GetForCurrentThread()->Bounds.Height, dpi);
		}
	}

	void* uwp_get_corewindow(void)
	{
		return (void*)CoreWindow::GetForCurrentThread();
	}

	void uwp_fill_installed_core_packages(struct string_list *list)
	{
		for (auto package : Windows::ApplicationModel::Package::Current->Dependencies)
		{
			if (package->IsOptional)
			{
				string_list_elem_attr attr{};
				string_list_append(list, utf16_to_utf8_string_alloc((package->InstalledLocation->Path + L"\\cores")->Data()), attr);
			}
		}
	}

	void uwp_input_next_frame(void *data)
	{
		uwp_current_input                = uwp_next_input;
		uwp_next_input.mouse_rel_x       = 0;
		uwp_next_input.mouse_rel_y       = 0;
		uwp_next_input.mouse_wheel_up   %= WHEEL_DELTA;
		uwp_next_input.mouse_wheel_left %= WHEEL_DELTA;
	}

	bool uwp_keyboard_pressed(unsigned key)
   {
      VirtualKey sym = (VirtualKey)kingsn_keysym_lut[(enum ks_key)key];

      if (sym == VirtualKey::None)
         return false;

      CoreWindow^ window = CoreWindow::GetForCurrentThread();

      /* At times CoreWindow will return NULL while running Dolphin core
       * Dolphin core runs on its own CPU thread separate from the UI-thread and so we must do a check for this. */
      if (!window)
         return false;
      return (window->GetKeyState(sym) & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;
   }

	int16_t uwp_mouse_state(unsigned port, unsigned id, bool screen)
   {
      int16_t state = 0;

      switch (id)
      {
         case KS_DEVICE_ID_MOUSE_X:
            return screen 
               ? uwp_current_input.mouse_screen_x 
               : uwp_current_input.mouse_rel_x;
         case KS_DEVICE_ID_MOUSE_Y:
            return screen 
               ? uwp_current_input.mouse_screen_y 
               : uwp_current_input.mouse_rel_y;
         case KS_DEVICE_ID_MOUSE_LEFT:
            return uwp_current_input.mouse_left;
         case KS_DEVICE_ID_MOUSE_RIGHT:
            return uwp_current_input.mouse_right;
         case KS_DEVICE_ID_MOUSE_WHEELUP:
            return uwp_current_input.mouse_wheel_up > WHEEL_DELTA;
         case KS_DEVICE_ID_MOUSE_WHEELDOWN:
            return uwp_current_input.mouse_wheel_up < -WHEEL_DELTA;
         case KS_DEVICE_ID_MOUSE_HORIZ_WHEELUP:
            return uwp_current_input.mouse_wheel_left > WHEEL_DELTA;
         case KS_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN:
            return uwp_current_input.mouse_wheel_left < -WHEEL_DELTA;
         case KS_DEVICE_ID_MOUSE_MIDDLE:
            return uwp_current_input.mouse_middle;
         case KS_DEVICE_ID_MOUSE_BUTTON_4:
            return uwp_current_input.mouse_button4;
         case KS_DEVICE_ID_MOUSE_BUTTON_5:
            return uwp_current_input.mouse_button5;
      }

      return 0;
   }

	int16_t uwp_pointer_state(unsigned idx, unsigned id, bool screen)
	{
		switch (id)
      {
         case KS_DEVICE_ID_POINTER_X:
            return screen 
               ? uwp_current_input.touch[idx].full_x 
               : uwp_current_input.touch[idx].x;
         case KS_DEVICE_ID_POINTER_Y:
            return screen 
               ? uwp_current_input.touch[idx].full_y 
               : uwp_current_input.touch[idx].y;
         case KS_DEVICE_ID_POINTER_PRESSED:
            return uwp_current_input.touch[idx].isInContact;
         case KS_DEVICE_ID_POINTER_COUNT:
            return uwp_current_input.touch_count;
         default:
            break;
      }

		return 0;
	}

	void uwp_open_broadfilesystemaccess_settings(void)
	{
		Windows::System::Launcher::LaunchUriAsync(ref new Uri("ms-settings:privacy-broadfilesystemaccess"));
	}

	enum ks_language uwp_get_language(void)
	{
		auto lang                 = Windows::System::UserProfile::GlobalizationPreferences::Languages->GetAt(0);
      struct string_list  split = {0};
		char lang_bcp[16]         = {0};
		char lang_iso[16]         = {0};

		wcstombs(lang_bcp, lang->Data(), sizeof(lang_bcp));

		/* Trying to convert BCP 47 language codes to ISO 639 ones */
      string_list_initialize(&split);
		string_split_noalloc(&split, lang_bcp, "-");

		strlcpy(lang_iso, split.elems[0].data, sizeof(lang_iso));

		if (split.size >= 2)
		{
			strlcat(lang_iso, "_", sizeof(lang_iso));
			strlcat(lang_iso, split.elems[split.size >= 3 ? 2 : 1].data,
               sizeof(lang_iso));
		}
      string_list_deinitialize(&split);
		return kingsn_get_language_from_iso(lang_iso);
	}

	const char *uwp_get_cpu_model_name(void)
	{
		Platform::String^ cpu_id = nullptr;
		Platform::String^ cpu_name = nullptr;
		
		/* GUID_DEVICE_PROCESSOR: {97FADB10-4E33-40AE-359C-8BEF029DBDD0} */
		Platform::String^ if_filter = L"System.Devices.InterfaceClassGuid:=\"{97FADB10-4E33-40AE-359C-8BEF029DBDD0}\"";

		/* Enumerate all CPU DeviceInterfaces, and get DeviceInstanceID of the first one. */
		cpu_id = RunAsyncAndCatchErrors<Platform::String^>([&]() {
			return create_task(DeviceInformation::FindAllAsync(if_filter)).then(
				[&](DeviceInformationCollection^ collection)
				{
					return dynamic_cast<Platform::String^>(
						collection->GetAt(0)->Properties->Lookup(L"System.Devices.DeviceInstanceID"));
				});
			}, nullptr);

		if (cpu_id)
		{
			Platform::String^ dev_filter = L"System.Devices.DeviceInstanceID:=\"" + cpu_id + L"\"";

			/* Get the Device with the same ID as the DeviceInterface
			 * Then get the name (description) of that Device
			 * We have to do this because the DeviceInterface we get doesn't have a proper description. */
			cpu_name = RunAsyncAndCatchErrors<Platform::String^>([&]() {
				return create_task(
					DeviceInformation::FindAllAsync(dev_filter, {}, DeviceInformationKind::Device)).then(
						[&](DeviceInformationCollection^ collection)
						{
							return cpu_name = collection->GetAt(0)->Name;
						});
				}, nullptr);
		}
		
		
		if (!cpu_name)
         return "Unknown";

      wcstombs(win32_cpu_model_name, cpu_name->Data(), sizeof(win32_cpu_model_name));
      return win32_cpu_model_name;
	}
}
