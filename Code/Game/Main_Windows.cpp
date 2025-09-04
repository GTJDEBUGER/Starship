#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <math.h>
#include <cassert>
#include <crtdbg.h>

// Self include
#include "PlayerShip.hpp"

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Eventually, we'll remove most OpenGL references out of Main_Win32.cpp
// Both of the following lines will eventually move to the top of Engine/Renderer/Renderer.cpp
//
#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Later we will move this useful macro to a more central place, e.g. Engine/Core/EngineCommon.hpp
//
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will eventually go away once we add a Window engine class later on.
// 
constexpr float CLIENT_ASPECT = 2.f; // By the Assignment 1 requirements, this should be 2:1 aspect ratio


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: We will move each of these items to its proper place, once that place is established later on
// 
bool g_isQuitting = false;							// ...becomes App::m_isQuitting instead
HWND g_hWnd = nullptr;								// ...becomes void* Window::m_windowHandle
HDC g_displayDeviceContext = nullptr;				// ...becomes void* Window::m_displayContext
HGLRC g_openGLRenderingContext = nullptr;			// ...becomes void* Renderer::m_apiRenderingContext
char const* APP_NAME = "SD1-A02: Starship Prototype";	// ...becomes ??? (Change this per project!)

//-----------------------------------------------------------------------------------------------
//Self-defined globals
PlayerShip* g_ship1 = nullptr;
PlayerShip* g_ship2 = nullptr;
PlayerShip* g_ship3 = nullptr;

bool g_isSlowDown = false;
bool g_isPause = false;
bool pauseTrigger = true;
bool g_singleStep = false;

//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called back by Windows whenever we tell it to (by calling DispatchMessage).
//
// #SD1ToDo: Eventually, we will move this function to a more appropriate place (Engine/Window/Window.cpp) later on...
//
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	switch( wmMessageCode )
	{
		// App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
		case WM_CLOSE:
		{
			g_isQuitting = true;
			return 0; // "Consumes" this message (tells Windows "okay, we handled it")
		}

		// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
		case WM_KEYDOWN:
		{
			unsigned char asKey = (unsigned char)wParam;

			if (asKey == 'T') {
				g_isSlowDown = true;
			}

			if (asKey == 'P' && pauseTrigger) {
				g_isPause = !g_isPause;
				pauseTrigger = false;
			}

			if (asKey == 'O') {
				g_singleStep = true;
			}

			if (asKey == 'Q') {
				g_isQuitting = true;
			}
			break;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			unsigned char asKey = (unsigned char) wParam;

			// #SD1ToDo: Tell the App (or InputSystem later) about this key-released event...
			if (asKey == 'T') {
				g_isSlowDown = false;
			}

			if(asKey == 'P' && !pauseTrigger) {
				pauseTrigger = true;
			}
			break;
		}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: We will move this function to a more appropriate place later on... (Engine/Window/Window.cpp)
//
void CreateOSWindow( void* applicationInstanceHandle, float clientAspect )
{
	SetProcessDpiAwarenessContext( DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 );

	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	DWORD const windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	DWORD const windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if( clientAspect > desktopAspect )
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / clientAspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * clientAspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[1024];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[0] ) );
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE) applicationInstanceHandle,
		NULL );

	ShowWindow( g_hWnd, SW_SHOW );
	SetForegroundWindow( g_hWnd );
	SetFocus( g_hWnd );

	g_displayDeviceContext = GetDC( g_hWnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );
}


//------------------------------------------------------------------------------------------------
// Given an existing OS Window, create a Rendering Context (RC) for OpenGL or DirectX to draw to it.
// #SD1ToDo: Move this to become Renderer::CreateRenderingContext() in Engine/Renderer/Renderer.cpp
// #SD1ToDo: By the end of SD1-A1, this function will be called from the function Renderer::Startup
//
void CreateRenderingContext()
{
	// Creates an OpenGL rendering context (RC) and binds it to the current window's device context (DC)
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize = sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	// These two OpenGL-like functions (wglCreateContext and wglMakeCurrent) will remain here for now.
	int pixelFormatCode = ChoosePixelFormat( g_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( g_displayDeviceContext );
	wglMakeCurrent( g_displayDeviceContext, g_openGLRenderingContext );

	// #SD1ToDo: move all OpenGL functions (including those below) to Renderer.cpp (only!)
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}



//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
// #SD1ToDo: Eventually, we will move this function to a more appropriate place later on... (Engine/Window/Window.cpp)
//
void RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		BOOL const wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move this function to Game/App.cpp and rename it to the App::App() constructor function
//
void App_Constructor( void* applicationInstanceHandle, char const* commandLineString )
{
	UNUSED( commandLineString );
	CreateOSWindow( applicationInstanceHandle, CLIENT_ASPECT );	// #SD1ToDo: this will eventually move to Window.cpp
	CreateRenderingContext();									// #SD1ToDo: this will move to Renderer.cpp, called by Renderer::Startup

	g_ship1 = new PlayerShip(Vec2(0, 30), Vec2(12, 0));
	g_ship2 = new PlayerShip(Vec2(0, 50), Vec2(20, 0));
	g_ship3 = new PlayerShip(Vec2(0, 70), Vec2(15, 0));
}


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move this function to Game/App.cpp and rename it to the App::~App() destructor function
//
void App_Destructor()
{
	delete g_ship1;
	g_ship1 = nullptr;
	delete g_ship2;
	g_ship2 = nullptr;
	delete g_ship3;
	g_ship3 = nullptr;
}


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will become  App::Update( float deltaSeconds )
void App_Update(float deltaSeconds)
{
	float originalDeltaSeconds = deltaSeconds;
	if (g_isSlowDown) {
		deltaSeconds *= 0.1f;
		originalDeltaSeconds = deltaSeconds;
	}
	if (g_isPause) {
		deltaSeconds = 0.f;
	}
	if (g_singleStep) {
		g_singleStep = false;
		deltaSeconds = originalDeltaSeconds;

		g_isPause = true;
		pauseTrigger = true;
	}

	g_ship1->Update(deltaSeconds);
	g_ship2->Update(deltaSeconds);
	g_ship3->Update(deltaSeconds);

	if (g_ship1->m_position.x >= 200 ||
		g_ship2->m_position.x >= 200 ||
		g_ship3->m_position.x >= 200) {
		g_isQuitting = true;
	}
}

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will become  App::Render() const
//
// Some simple OpenGL example drawing code.
// This is the graphical equivalent of printing "Hello, world."
// #SD1ToDo: Move this function to Game/App.cpp and rename it to  App::Render() const
// #SD1ToDo: Move *ALL* OpenGL code to Renderer.cpp (only).
//
// Ultimately this function (App::Render) will only call methods on Renderer (like Renderer::DrawVertexArray)
//	to draw things, never calling OpenGL (nor DirectX) functions directly.
//
void App_Render()
{
	// Establish a 2D (orthographic) drawing coordinate system: (0,0) bottom-left to (10,10) top-right
	// #SD1ToDo: This will be replaced by a call to g_renderer->BeginView( m_worldView ); or similar
	glLoadIdentity();
	glOrtho( 0.f, 200.f, 0.f, 100.f, 0.f, 1.f ); // arguments are: xLeft, xRight, yBottom, yTop, zNear, zFar

	// Clear all screen (backbuffer) pixels to medium-blue
	// #SD1ToDo: This will become g_renderer->ClearColor( Rgba8( 0, 0, 127, 255 ) );
	// By the Assignment 1 requirements, the clear color must be (100,50,0), in noramalized floats that's (0.39, 0.20, 0.0)
	glClearColor( 0.39f, 0.2f, 0.f, 1.f ); // Note; glClearColor takes colors as floats in [0,1], not bytes in [0,255]
	glClear( GL_COLOR_BUFFER_BIT ); // ALWAYS clear the screen at the top of each frame's Render()!

	// Draw some triangles (provide 3 vertexes each)
	// #SD1ToDo: Move all OpenGL code into Renderer.cpp (only); call g_renderer->DrawVertexArray() instead
	glBegin( GL_TRIANGLES );
	{
		g_ship1->Render();
		g_ship2->Render();
		g_ship3->Render();
	}
	glEnd();	
}


//-----------------------------------------------------------------------------------------------
// #SD1ToDo: This will become  App::Run()
void App_Run()
{
	// Program main loop; keep running frames until it's time to quit
	while( !g_isQuitting )			// #SD1ToDo: ...becomes:  !g_theApp->IsQuitting()
	{
		// #SD1ToDo: This call will move to Window::BeginFrame() once we have a Window engine system
		// Process OS messages (keyboard/mouse button clicked, application lost/gained focus, etc.)
		RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!

		float fakeDeltaSeconds = 1.f / 60.f;
		// One "frame" of the game.  Generally: Input, Update, Render.  We call this 60+ times per second.
		// g_engine->BeginFrame(); // Allow engine subsystems to do pre-frame stuff
		App_Update(fakeDeltaSeconds);		// #SD1ToDo: ...becomes just Update();		once this function becomes App::Run()
		App_Render();		// #SD1ToDo: ...becomes just Render();		once this function becomes App::Run()
		// g_engine->EndFrame(); // Allow engine subsystems to do post-frame stuff

		Sleep( 16 ); // Temporary code to "slow down" our app to ~60Hz until we have proper frame timing in

		// #SD1ToDo: This call will move to Renderer::EndFrame() once we complete our Window refactor
		// "Present" the backbuffer by swapping the front (visible) and back (working) screen buffers
		SwapBuffers( g_displayDeviceContext ); // Note: call this only once at the very end of each frame
	}
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED(applicationInstanceHandle);
	UNUSED( commandLineString );

	App_Constructor( applicationInstanceHandle, commandLineString ); // This will get replaced with:
	// #SD1ToDo: g_theApp = new App();

	App_Run(); // This will get replaced with:
	// #SD1ToDo: g_theApp->Run();

	App_Destructor(); // This will get replaced with:
	// #SD1ToDo:	delete g_theApp;
	// #SD1ToDo:	g_theApp = nullptr;

	return 0;
}


