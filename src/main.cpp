#include <windows.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <winhttp.h>
#include <commctrl.h>
#include <string>
#include <fstream>
#include <vector>
#include "resource.h"
#include "upload.h"
#include "overlay.h"
#include "toast.h"

#pragma comment(lib, "comctl32.lib")

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
int g_hotkeyRegionId = 2;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
void ShowSettingsDialog();
void CaptureScreenshot(bool useRegion = false);
void ShowToast(const std::wstring& url);
std::vector<BYTE> CaptureRegion(const RECT& rect);
std::vector<BYTE> CaptureFullscreen();

// System tray menu IDs
#define IDM_CAPTURE_FULL 1001
#define IDM_CAPTURE_REGION 1002
#define IDM_SETTINGS 1003
#define IDM_EXIT 1004
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
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
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
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon
    wcscpy_s(g_nid.szTip, L"Nekoo Screenshot - Ctrl+Shift+S");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
    
    // Register global hotkeys
    RegisterHotKey(g_hwndMain, g_hotkeyId, MOD_CONTROL | MOD_SHIFT, 'S'); // Fullscreen
    RegisterHotKey(g_hwndMain, g_hotkeyRegionId, MOD_CONTROL | MOD_SHIFT, 'R'); // Region
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    UnregisterHotKey(g_hwndMain, g_hotkeyId);
    UnregisterHotKey(g_hwndMain, g_hotkeyRegionId);
    GdiplusShutdown(g_gdiplusToken);
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            if (wParam == g_hotkeyId) {
                CaptureScreenshot(false);
            } else if (wParam == g_hotkeyRegionId) {
                CaptureScreenshot(true);
            }
            return 0;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, IDM_CAPTURE_FULL, L"Capture Fullscreen (Ctrl+Shift+S)");
                AppendMenu(hMenu, MF_STRING, IDM_CAPTURE_REGION, L"Capture Region (Ctrl+Shift+R)");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
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
                case IDM_CAPTURE_FULL:
                    CaptureScreenshot(false);
                    break;
                case IDM_CAPTURE_REGION:
                    CaptureScreenshot(true);
                    break;
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

std::vector<BYTE> CaptureFullscreen() {
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ hOld = SelectObject(hDC, hBmp);
    BitBlt(hDC, 0, 0, w, h, hScreen, 0, 0, SRCCOPY);
    
    Bitmap* bmp = new Bitmap(hBmp, NULL);
    IStream* stream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);
    
    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    bmp->Save(stream, &clsid);
    
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
    SelectObject(hDC, hOld);
    DeleteObject(hBmp);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    
    return data;
}

std::vector<BYTE> CaptureRegion(const RECT& rect) {
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    
    if (w <= 0 || h <= 0) return std::vector<BYTE>();
    
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ hOld = SelectObject(hDC, hBmp);
    BitBlt(hDC, 0, 0, w, h, hScreen, rect.left, rect.top, SRCCOPY);
    
    Bitmap* bmp = new Bitmap(hBmp, NULL);
    IStream* stream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);
    
    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    bmp->Save(stream, &clsid);
    
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
    SelectObject(hDC, hOld);
    DeleteObject(hBmp);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    
    return data;
}

void CaptureScreenshot(bool useRegion) {
    std::vector<BYTE> imageData;
    
    if (useRegion) {
        RECT rect = {0};
        ShowOverlay(g_hwndMain, &rect);
        
        if (rect.right == 0 && rect.bottom == 0) {
            return; // Cancelled
        }
        
        imageData = CaptureRegion(rect);
    } else {
        imageData = CaptureFullscreen();
    }
    
    if (imageData.empty()) {
        MessageBox(NULL, L"Failed to capture screenshot", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    // Upload to nekoo.ru
    std::wstring url = UploadToNekoo(imageData);
    
    if (url.empty()) {
        MessageBox(NULL, L"Failed to upload screenshot", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    ShowToast(url);
}

void ShowToast(const std::wstring& url) {
    ShowToastNotification(url);
}

void ShowSettingsDialog() {
    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SETTINGS), g_hwndMain, SettingsDlgProc);
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            // Set current hotkey
            SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, 
                MAKEWORD('S', HOTKEYF_CONTROL | HOTKEYF_SHIFT), 0);
            
            // Check auto-start checkbox
            CheckDlgButton(hDlg, IDC_AUTOSTART, BST_UNCHECKED);
            return TRUE;
        
        case WM_CTLCOLORDLG:
            return (LRESULT)CreateSolidBrush(RGB(26, 26, 26));
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(255, 255, 255));
            SetBkColor(hdcStatic, RGB(26, 26, 26));
            return (LRESULT)CreateSolidBrush(RGB(26, 26, 26));
        }
        
        case WM_CTLCOLORBTN: {
            HDC hdcButton = (HDC)wParam;
            SetTextColor(hdcButton, RGB(255, 255, 255));
            SetBkColor(hdcButton, RGB(139, 92, 246));
            return (LRESULT)CreateSolidBrush(RGB(139, 92, 246));
        }
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    // TODO: Save settings
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                    
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}
