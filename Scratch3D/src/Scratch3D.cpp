#include "Defines.h"
#include "Types.h"

#include <strsafe.h>
#include <Windows.h>

union Pixel {
  struct {
    U8 b, g, r, a;
  };
  U32 color;

  Pixel() : r(0), g(0), b(0), a(0) {}
  Pixel(U8 r, U8 g, U8 b, U8 a = 255) : r(r), g(g), b(b), a(a) {}
  Pixel(U32 c) : color(c) {}
  operator U32() const { return color; }
};

HINSTANCE g_Instance;
HWND g_Window;
HDC g_DeviceContext;
bool g_CloseRequested = false;

static const U32 g_WindowWidth = 1280;
static const U32 g_WindowHeight = 720;
static const wchar_t* g_WindowClassName = L"Scratch3D";

static Pixel g_Framebuffers[2][g_WindowWidth * g_WindowHeight]{};
static U8 g_CurrentFramebuffer = 0;
static const U8 g_MaxFramebuffers = 2;

LARGE_INTEGER microsecondsLastFrame;

static BITMAPINFO g_BitmapInfo{};

void Draw() {
  static U8 strength = 255;
  static bool inc = false;

  if (strength == 255) {
    inc = false;
  } else if (strength == 0) {
    inc = true;
  }
  strength = strength + (inc ? 1 : -1);

  g_CurrentFramebuffer = (g_CurrentFramebuffer + 1) % g_MaxFramebuffers;
  auto& fb = g_Framebuffers[g_CurrentFramebuffer];

  memset(&fb, 0, sizeof(fb));
  for (U32 y = 0; y < g_WindowHeight; y++) {
    for (U32 x = 0; x < g_WindowWidth; x++) {
      auto& px = fb[(y * g_WindowWidth) + x];
      px.r = strength;
    }
  }
}

LRESULT CALLBACK ApplicationWindowProcedure(HWND hwnd, U32 msg, WPARAM wParam, LPARAM lParam) {
  const U32 bufferSize = 32;
  TCHAR dstStr[bufferSize];
  size_t dstSize = bufferSize * sizeof(TCHAR);
  LPCTSTR format = TEXT("%.2f ms / %.1f FPS");
  ASSERT(SUCCEEDED(
      StringCbPrintfA(dstStr, dstSize, format, (F32)microsecondsLastFrame.QuadPart / 1000.0f,
                      (F32)1000.0f / ((F32)microsecondsLastFrame.QuadPart / 1000.0f))));

  switch (msg) {
    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC context = BeginPaint(g_Window, &paint);

      Draw();

      ASSERT(StretchDIBits(context, 0, 0, g_WindowWidth, g_WindowHeight, 0, 0, g_WindowWidth,
                    g_WindowHeight, g_Framebuffers[g_CurrentFramebuffer], &g_BitmapInfo, DIB_RGB_COLORS, SRCCOPY));
      TextOutA(context, 0, 0, dstStr, lstrlenA(dstStr));

      EndPaint(g_Window, &paint);
      return 0;
    }
    case WM_CLOSE:
      g_CloseRequested = true;
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ProcessEvents() {
  MSG message;
  while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageW(&message);
  }
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  WNDCLASSW wc{};
  wc.lpfnWndProc = ApplicationWindowProcedure;
  wc.hInstance = g_Instance;
  wc.lpszClassName = g_WindowClassName;
  RegisterClassW(&wc);

  U32 windowStyle =
      WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  U32 windowExStyle = WS_EX_APPWINDOW;

  U32 windowW = g_WindowWidth;
  U32 windowH = g_WindowHeight;
  U32 windowX = 100;
  U32 windowY = 100;

  RECT borderRect = {0, 0, 0, 0};
  AdjustWindowRectEx(&borderRect, windowStyle, false, windowExStyle);
  windowX += borderRect.left;
  windowY += borderRect.top;
  windowW += borderRect.right - borderRect.left;
  windowH += borderRect.bottom - borderRect.top;

  g_Window = CreateWindowExW(windowExStyle, g_WindowClassName, L"Scratch3D", windowStyle, windowX,
                             windowY, windowW, windowH, nullptr, nullptr, g_Instance, nullptr);
  if (!g_Window) {
    ASSERT(false);
  }

  ShowWindow(g_Window, SW_SHOW);

  g_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
  g_BitmapInfo.bmiHeader.biWidth = g_WindowWidth;
  g_BitmapInfo.bmiHeader.biHeight = -static_cast<I32>(g_WindowHeight);
  g_BitmapInfo.bmiHeader.biPlanes = 1;
  g_BitmapInfo.bmiHeader.biBitCount = 32;
  g_BitmapInfo.bmiHeader.biCompression = BI_RGB;

  g_DeviceContext = GetDC(g_Window);

  const RECT windowRect = {0, 0, g_WindowWidth, g_WindowHeight};
  LARGE_INTEGER queryFreq;
  LARGE_INTEGER startTime, endTime, elapsedTime;
  QueryPerformanceFrequency(&queryFreq);
  QueryPerformanceCounter(&startTime);
  while (!g_CloseRequested) {
    InvalidateRect(g_Window, &windowRect, true);
    ASSERT(RedrawWindow(g_Window, &windowRect, NULL, RDW_INTERNALPAINT));
    ProcessEvents();
    QueryPerformanceCounter(&endTime);
    elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
    elapsedTime.QuadPart = (elapsedTime.QuadPart * 1000000) / queryFreq.QuadPart;
    microsecondsLastFrame = elapsedTime;
    startTime = endTime;
  }

  ReleaseDC(g_Window, g_DeviceContext);
  DestroyWindow(g_Window);

  return 0;
}