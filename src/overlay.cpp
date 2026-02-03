#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include "overlay.h"

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

static HWND g_hwndOverlay = NULL;
static POINT g_ptStart = {0};
static POINT g_ptEnd = {0};
static bool g_bSelecting = false;
static RECT g_selectedRect = {0};
static bool g_bCancelled = false;

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ShowOverlay(HWND hwndParent, RECT* pRect) {
    // Register overlay window class
    static bool registered = false;
    if (!registered) {
        WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
        wc.lpfnWndProc = OverlayWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = L"NekooOverlayClass";
        wc.hCursor = LoadCursor(NULL, IDC_CROSS);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        RegisterClassEx(&wc);
        registered = true;
    }
    
    // Reset state
    g_bSelecting = false;
    g_bCancelled = false;
    g_ptStart = {0};
    g_ptEnd = {0};
    
    // Get screen dimensions
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    
    // Create fullscreen overlay
    g_hwndOverlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        L"NekooOverlayClass", L"",
        WS_POPUP,
        0, 0, w, h,
        hwndParent, NULL, GetModuleHandle(NULL), NULL);
    
    // Make it semi-transparent
    SetLayeredWindowAttributes(g_hwndOverlay, 0, 100, LWA_ALPHA);
    
    // Remove transparency for mouse input
    SetWindowLong(g_hwndOverlay, GWL_EXSTYLE,
        GetWindowLong(g_hwndOverlay, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
    
    ShowWindow(g_hwndOverlay, SW_SHOW);
    SetCapture(g_hwndOverlay);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.hwnd == g_hwndOverlay || msg.message == WM_QUIT) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (!IsWindow(g_hwndOverlay)) break;
        }
    }
    
    if (g_bCancelled || !g_bSelecting) {
        pRect->left = pRect->top = pRect->right = pRect->bottom = 0;
    } else {
        *pRect = g_selectedRect;
    }
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_LBUTTONDOWN:
            g_bSelecting = true;
            g_ptStart.x = GET_X_LPARAM(lParam);
            g_ptStart.y = GET_Y_LPARAM(lParam);
            g_ptEnd = g_ptStart;
            return 0;
            
        case WM_MOUSEMOVE:
            if (g_bSelecting) {
                g_ptEnd.x = GET_X_LPARAM(lParam);
                g_ptEnd.y = GET_Y_LPARAM(lParam);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
            
        case WM_LBUTTONUP:
            if (g_bSelecting) {
                g_ptEnd.x = GET_X_LPARAM(lParam);
                g_ptEnd.y = GET_Y_LPARAM(lParam);
                
                // Calculate selected rectangle
                g_selectedRect.left = min(g_ptStart.x, g_ptEnd.x);
                g_selectedRect.top = min(g_ptStart.y, g_ptEnd.y);
                g_selectedRect.right = max(g_ptStart.x, g_ptEnd.x);
                g_selectedRect.bottom = max(g_ptStart.y, g_ptEnd.y);
                
                ReleaseCapture();
                DestroyWindow(hwnd);
            }
            return 0;
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                g_bCancelled = true;
                ReleaseCapture();
                DestroyWindow(hwnd);
            }
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Draw semi-transparent overlay
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            Graphics graphics(hdc);
            SolidBrush darkBrush(Color(100, 0, 0, 0));
            graphics.FillRectangle(&darkBrush, 0, 0, rc.right, rc.bottom);
            
            // Draw selection rectangle
            if (g_bSelecting) {
                int x = min(g_ptStart.x, g_ptEnd.x);
                int y = min(g_ptStart.y, g_ptEnd.y);
                int w = abs(g_ptEnd.x - g_ptStart.x);
                int h = abs(g_ptEnd.y - g_ptStart.y);
                
                // Clear selected area
                graphics.SetCompositingMode(CompositingModeSourceCopy);
                SolidBrush clearBrush(Color(0, 0, 0, 0));
                graphics.FillRectangle(&clearBrush, x, y, w, h);
                
                // Draw border
                Pen borderPen(Color(255, 139, 92, 246), 2); // Purple
                graphics.DrawRectangle(&borderPen, x, y, w, h);
                
                // Draw dimensions text
                wchar_t text[64];
                _snwprintf_s(text, _TRUNCATE, L"%d x %d", w, h);
                
                Font font(L"Segoe UI", 12);
                SolidBrush textBrush(Color(255, 255, 255, 255));
                graphics.DrawString(text, -1, &font, PointF((float)x, (float)y - 20), &textBrush);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_DESTROY:
            g_hwndOverlay = NULL;
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
