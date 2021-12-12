#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void
RenderWeirdGradient(int XOffset, int YOffset)
{
	int Width 		= BitmapWidth;
	int Height 		= BitmapHeight;

	int Pitch 		= Width*BytesPerPixel;
	u8 *Row 		= (u8*) BitmapMemory;

	for (int Y = 0; Y < BitmapHeight; ++Y)
	{
		u32 *Pixel = (u32*)Row;

		for (int X = 0; X < BitmapWidth; ++X)
		{
			u8 Blue = (X + XOffset);
			u8 Green = (Y + YOffset);

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Pitch;
	}
}

internal void
Win32ResizeDIBSection(int Width, int Height)
{	
	if (BitmapMemory)
	{
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	}

	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	
	int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
	BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;

	StretchDIBits(DeviceContext,
		/*X, Y, Width, Height,
		X, Y, Width, Height,*/
		0, 0, BitmapWidth, BitmapHeight,
		0, 0, WindowWidth, WindowHeight,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
MainWindowCallback(
	HWND Window, 
	UINT Message,
	WPARAM WParam, 
	LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			Win32ResizeDIBSection(Width, Height);
			OutputDebugStringA("WM_SIZE\n");
		} break;
		case WM_DESTROY:
		{
			Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		} break;
		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE\n");
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			Win32UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);
			EndPaint(Window, &Paint);
		} break;
		default: 
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return (Result);
}


int CALLBACK
WinMain(HINSTANCE 	Instance,
	HINSTANCE 	  	PrevInstance, 
	LPSTR 			CommandLine,
	int 			ShowCode)
{
	
	WNDCLASS WindowClass = {};

	WindowClass.style 		= CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance 	= Instance;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&WindowClass))
	{
		HWND Window = 
			CreateWindowEx(
				0,
				WindowClass.lpszClassName,
				"Handmade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0
			);

			if (Window)
			{
				Running = true;
				while (Running)
				{
					int XOffset = 0;
					int YOffset = 0;

					MSG Message;
					while (PeekMessage(&Message, 0,0,0,PM_REMOVE))
					{
						if (Message.message = WM_QUIT)
						{
							Running = false;
						}
						TranslateMessage(&Message);
						DispatchMessage(&Message);
					}

					RenderWeirdGradient(XOffset, YOffset);
					HDC DeviceContext = GetDC(Window);
					RECT ClientRect;
					GetClientRect(Window, &ClientRect);
					int WindowWidth = ClientRect.right - ClientRect.left;
					int WindowHeight = ClientRect.bottom - ClientRect.top;

					Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
					ReleaseDC(Window, DeviceContext);

					++XOffset;
				}
				
			}
			else
			{

			}
	}
	else 
	{

	}

	return (0);
}
