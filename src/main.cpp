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
#include "settings.h"

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
AppSettings g_settings;

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
void ShowSettingsDialog();
void CaptureScreenshot(bool useRegion = false);
void ShowToast(const std::wstring& url);
std::vector<BYTE> CaptureRegion(const RECT& rect);
std::vector<BYTE> CaptureFullscreen();
void RegisterHotkeys();
void UnregisterHotkeys();
UINT ModifiersFromHotkey(WORD hotkey);
UINT KeyFromHotkey(WORD hotkey);

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
    
    // Load settings
    g_settings = LoadSettings();
    
    // Create system tray icon
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hwndMain;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Default icon
    wcscpy_s(g_nid.szTip, L"Nekoo Screenshot");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
    
    // Register global hotkeys from settings
    RegisterHotkeys();
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
    UnregisterHotkeys();
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
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
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
    
    // Show "Uploading..." toast immediately
    ShowUploadingToast();
    
    // Upload to nekoo.ru
    std::wstring url = UploadToNekoo(imageData);
    
    if (url.empty()) {
        CloseToast();
        MessageBox(NULL, L"Failed to upload screenshot", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    // Update toast with URL
    UpdateToastWithUrl(url);
}

void ShowToast(const std::wstring& url) {
    ShowToastNotification(url);
}

void ShowSettingsDialog() {
    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SETTINGS), g_hwndMain, SettingsDlgProc);
}

UINT ModifiersFromHotkey(WORD hotkey) {
    UINT mods = 0;
    if (HIBYTE(hotkey) & HOTKEYF_CONTROL) mods |= MOD_CONTROL;
    if (HIBYTE(hotkey) & HOTKEYF_SHIFT) mods |= MOD_SHIFT;
    if (HIBYTE(hotkey) & HOTKEYF_ALT) mods |= MOD_ALT;
    return mods;
}

UINT KeyFromHotkey(WORD hotkey) {
    return LOBYTE(hotkey);
}

void RegisterHotkeys() {
    RegisterHotKey(g_hwndMain, g_hotkeyId, g_settings.fullscreenModifiers, g_settings.fullscreenKey);
    RegisterHotKey(g_hwndMain, g_hotkeyRegionId, g_settings.regionModifiers, g_settings.regionKey);
}

void UnregisterHotkeys() {
    UnregisterHotKey(g_hwndMain, g_hotkeyId);
    UnregisterHotKey(g_hwndMain, g_hotkeyRegionId);
}

INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG: {
            // Allow hotkeys without modifiers (like PrintScreen alone)
            SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETRULES, 
                HKCOMB_NONE | HKCOMB_S, 
                MAKELPARAM(HOTKEYF_CONTROL | HOTKEYF_ALT, 0));
            SendDlgItemMessage(hDlg, IDC_HOTKEY_REGION, HKM_SETRULES, 
                HKCOMB_NONE | HKCOMB_S, 
                MAKELPARAM(HOTKEYF_CONTROL | HOTKEYF_ALT, 0));
            
            // Set fullscreen hotkey
            BYTE mods = 0;
            if (g_settings.fullscreenModifiers & MOD_CONTROL) mods |= HOTKEYF_CONTROL;
            if (g_settings.fullscreenModifiers & MOD_SHIFT) mods |= HOTKEYF_SHIFT;
            if (g_settings.fullscreenModifiers & MOD_ALT) mods |= HOTKEYF_ALT;
            SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, 
                MAKEWORD(g_settings.fullscreenKey, mods), 0);
            
            // Set region hotkey
            mods = 0;
            if (g_settings.regionModifiers & MOD_CONTROL) mods |= HOTKEYF_CONTROL;
            if (g_settings.regionModifiers & MOD_SHIFT) mods |= HOTKEYF_SHIFT;
            if (g_settings.regionModifiers & MOD_ALT) mods |= HOTKEYF_ALT;
            SendDlgItemMessage(hDlg, IDC_HOTKEY_REGION, HKM_SETHOTKEY, 
                MAKEWORD(g_settings.regionKey, mods), 0);
            
            // Set auto-start checkbox
            CheckDlgButton(hDlg, IDC_AUTOSTART, g_settings.autoStart ? BST_CHECKED : BST_UNCHECKED);
            return TRUE;
        }
        
        case WM_CTLCOLORDLG:
            // Clean white background
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(50, 50, 50)); // Dark gray text
            SetBkColor(hdcStatic, RGB(255, 255, 255)); // White background
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;
            SetTextColor(hdcEdit, RGB(30, 30, 30));
            SetBkColor(hdcEdit, RGB(250, 250, 250));
            return (LRESULT)CreateSolidBrush(RGB(250, 250, 250));
        }
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: {
                    // Get fullscreen hotkey
                    WORD hotkey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);
                    g_settings.fullscreenModifiers = ModifiersFromHotkey(hotkey);
                    g_settings.fullscreenKey = KeyFromHotkey(hotkey);
                    
                    // Get region hotkey
                    hotkey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTKEY_REGION, HKM_GETHOTKEY, 0, 0);
                    g_settings.regionModifiers = ModifiersFromHotkey(hotkey);
                    g_settings.regionKey = KeyFromHotkey(hotkey);
                    
                    // Get auto-start
                    g_settings.autoStart = (IsDlgButtonChecked(hDlg, IDC_AUTOSTART) == BST_CHECKED);
                    
                    // Save settings
                    SaveSettings(g_settings);
                    
                    // Re-register hotkeys
                    UnregisterHotkeys();
                    RegisterHotkeys();
                    
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                }
                    
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

