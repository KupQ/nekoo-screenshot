#include <windows.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <winhttp.h>
#include <string>
#include <fstream>
#include "resource.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "shell32.lib")

using namespace Gdiplus;

// Global variables
HINSTANCE g_hInst;
NOTIFYICONDATA g_nid;
HWND g_hwndMain;
ULONG_PTR g_gdiplusToken;
int g_hotkeyId = 1;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
void ShowSettingsDialog();
void CaptureScreenshot();
void ShowToast(const std::wstring& url);

// System tray menu IDs
#define IDM_SETTINGS 1001
#define IDM_EXIT 1002
#define WM_TRAYICON (WM_USER + 1)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInst = hInstance;
    
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    
    // Register window class
    WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"NekooScreenshotClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    RegisterClassEx(&wc);
    
    // Create hidden window
    g_hwndMain = CreateWindowEx(0, L"NekooScreenshotClass", L"Nekoo Screenshot",
        WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    
    // Create system tray icon
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hwndMain;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wcscpy_s(g_nid.szTip, L"Nekoo Screenshot - Ctrl+Shift+S");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
    
    // Register global hotkey (Ctrl+Shift+S)
    RegisterHotKey(g_hwndMain, g_hotkeyId, MOD_CONTROL | MOD_SHIFT, 'S');
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    UnregisterHotKey(g_hwndMain, g_hotkeyId);
    GdiplusShutdown(g_gdiplusToken);
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            if (wParam == g_hotkeyId) {
                CaptureScreenshot();
            }
            return 0;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, IDM_SETTINGS, L"Settings");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");
                
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                    pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_SETTINGS:
                    ShowSettingsDialog();
                    break;
                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CaptureScreenshot() {
    // Get screen dimensions
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    
    // Capture screen
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, w, h);
    SelectObject(hDC, hBmp);
    BitBlt(hDC, 0, 0, w, h, hScreen, 0, 0, SRCCOPY);
    
    // Convert to PNG
    Bitmap* bmp = new Bitmap(hBmp, NULL);
    IStream* stream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);
    
    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    bmp->Save(stream, &clsid);
    
    // Get data
    STATSTG stat;
    stream->Stat(&stat, STATFLAG_DEFAULT);
    ULONG size = (ULONG)stat.cbSize.QuadPart;
    
    std::vector<BYTE> data(size);
    LARGE_INTEGER pos = {0};
    stream->Seek(pos, STREAM_SEEK_SET, NULL);
    ULONG bytesRead;
    stream->Read(data.data(), size, &bytesRead);
    stream->Release();
    
    delete bmp;
    DeleteObject(hBmp);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    
    // Upload (simplified for now)
    ShowToast(L"https://nekoo.ru/test123");
}

void ShowToast(const std::wstring& url) {
    // Simple message box for now - will be replaced with custom toast
    MessageBox(NULL, url.c_str(), L"Screenshot Uploaded", MB_OK | MB_ICONINFORMATION);
    
    // Copy to clipboard
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        size_t size = (url.length() + 1) * sizeof(wchar_t);
        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
        if (h) {
            memcpy(GlobalLock(h), url.c_str(), size);
            GlobalUnlock(h);
            SetClipboardData(CF_UNICODETEXT, h);
        }
        CloseClipboard();
    }
}

void ShowSettingsDialog() {
    MessageBox(NULL, L"Settings dialog coming soon!", L"Settings", MB_OK);
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Will be implemented
    return FALSE;
}
