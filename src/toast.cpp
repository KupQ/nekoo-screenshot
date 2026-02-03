#include <windows.h>
#include <string>
#include "toast.h"

static HWND g_hwndToast = NULL;
static std::wstring g_toastUrl;
static UINT_PTR g_timerId = 0;

#define TOAST_WIDTH 400
#define TOAST_HEIGHT 100
#define TOAST_MARGIN 20
#define TOAST_TIMER_ID 1

LRESULT CALLBACK ToastWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ShowToastNotification(const std::wstring& url) {
    g_toastUrl = url;
    
    // Register toast window class
    static bool registered = false;
    if (!registered) {
        WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
        wc.lpfnWndProc = ToastWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"NekooToastClass";
        wc.hbrBackground = CreateSolidBrush(RGB(26, 26, 26)); // Dark background
        RegisterClassEx(&wc);
        registered = true;
    }
    
    // Get screen dimensions
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    
    // Position at bottom-right
    int x = screenW - TOAST_WIDTH - TOAST_MARGIN;
    int y = screenH - TOAST_HEIGHT - TOAST_MARGIN - 40; // Above taskbar
    
    // Create toast window
    g_hwndToast = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        L"NekooToastClass",
        L"",
        WS_POPUP,
        x, y, TOAST_WIDTH, TOAST_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    
    // Make it slightly transparent
    SetLayeredWindowAttributes(g_hwndToast, 0, 250, LWA_ALPHA);
    
    ShowWindow(g_hwndToast, SW_SHOW);
    
    // Set auto-dismiss timer (5 seconds)
    g_timerId = SetTimer(g_hwndToast, TOAST_TIMER_ID, 5000, NULL);
}

LRESULT CALLBACK ToastWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create "Copy" button
            CreateWindow(L"BUTTON", L"Copy Link",
                WS_CHILD | WS_VISIBLE | BS_FLAT,
                TOAST_WIDTH - 110, TOAST_HEIGHT - 40,
                100, 30,
                hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Set dark background
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, CreateSolidBrush(RGB(26, 26, 26)));
            
            // Draw purple border
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(139, 92, 246));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, 0, 0, rc.right, rc.bottom);
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);
            
            // Draw title
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            
            RECT titleRect = {10, 10, rc.right - 10, 30};
            DrawText(hdc, L"✅ Screenshot Uploaded!", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            
            // Draw URL
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(139, 92, 246)); // Purple
            
            RECT urlRect = {10, 35, rc.right - 120, 55};
            DrawText(hdc, g_toastUrl.c_str(), -1, &urlRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            
            SelectObject(hdc, hOldFont);
            DeleteObject(hFont);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                // Copy button clicked
                if (OpenClipboard(hwnd)) {
                    EmptyClipboard();
                    size_t size = (g_toastUrl.length() + 1) * sizeof(wchar_t);
                    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
                    if (h) {
                        memcpy(GlobalLock(h), g_toastUrl.c_str(), size);
                        GlobalUnlock(h);
                        SetClipboardData(CF_UNICODETEXT, h);
                    }
                    CloseClipboard();
                }
                
                // Close toast
                if (g_timerId) {
                    KillTimer(hwnd, g_timerId);
                    g_timerId = 0;
                }
                DestroyWindow(hwnd);
            }
            return 0;
        
        case WM_TIMER:
            if (wParam == TOAST_TIMER_ID) {
                KillTimer(hwnd, g_timerId);
                g_timerId = 0;
                DestroyWindow(hwnd);
            }
            return 0;
        
        case WM_LBUTTONDOWN:
            // Click anywhere to dismiss
            if (g_timerId) {
                KillTimer(hwnd, g_timerId);
                g_timerId = 0;
            }
            DestroyWindow(hwnd);
            return 0;
        
        case WM_DESTROY:
            g_hwndToast = NULL;
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
