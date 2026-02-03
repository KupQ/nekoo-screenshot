#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")

using namespace Gdiplus;

// Global hotkey ID
#define HOTKEY_SCREENSHOT 1

// Settings structure
struct Settings {
    int hotkeyModifiers = MOD_CONTROL | MOD_SHIFT;
    int hotkeyKey = 'S';
    std::wstring uploadUrl = L"nekoo.ru";
};

Settings g_settings;

// Function to capture screenshot
std::vector<BYTE> CaptureScreenshot() {
    std::vector<BYTE> imageData;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    SelectObject(hdcMem, hBitmap);
    
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
    
    Bitmap bitmap(hBitmap, NULL);
    IStream* stream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);
    
    CLSID pngClsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &pngClsid);
    bitmap.Save(stream, &pngClsid, NULL);
    
    STATSTG statstg;
    stream->Stat(&statstg, STATFLAG_DEFAULT);
    ULONG size = statstg.cbSize.LowPart;
    
    imageData.resize(size);
    LARGE_INTEGER li = {0};
    stream->Seek(li, STREAM_SEEK_SET, NULL);
    stream->Read(imageData.data(), size, NULL);
    stream->Release();
    
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return imageData;
}

// Function to upload to nekoo.ru
std::wstring UploadToNekoo(const std::vector<BYTE>& imageData) {
    std::wstring url;
    
    HINTERNET hSession = WinHttpOpen(L"Nekoo Screenshot/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    
    if (!hSession) return L"";
    
    HINTERNET hConnect = WinHttpConnect(hSession, g_settings.uploadUrl.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT, 0);
    
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return L"";
    }
    
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/upload",
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"";
    }
    
    std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::string contentType = "multipart/form-data; boundary=" + boundary;
    
    std::ostringstream body;
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"file\"; filename=\"screenshot.png\"\r\n";
    body << "Content-Type: image/png\r\n\r\n";
    body.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());
    body << "\r\n--" << boundary << "--\r\n";
    
    std::string bodyStr = body.str();
    
    std::wstring headers = L"Content-Type: " + std::wstring(contentType.begin(), contentType.end());
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
    
    BOOL result = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)bodyStr.c_str(), bodyStr.length(),
        bodyStr.length(), 0);
    
    if (result) {
        result = WinHttpReceiveResponse(hRequest, NULL);
        
        if (result) {
            DWORD dwSize = 0;
            std::string response;
            
            do {
                dwSize = 0;
                WinHttpQueryDataAvailable(hRequest, &dwSize);
                
                if (dwSize > 0) {
                    char* buffer = new char[dwSize + 1];
                    ZeroMemory(buffer, dwSize + 1);
                    DWORD dwDownloaded = 0;
                    
                    WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded);
                    response += buffer;
                    delete[] buffer;
                }
            } while (dwSize > 0);
            
            size_t pos = response.find("https://nekoo.ru/");
            if (pos != std::string::npos) {
                size_t end = response.find_first_of("\"'< ", pos);
                std::string urlStr = response.substr(pos, end - pos);
                url = std::wstring(urlStr.begin(), urlStr.end());
            }
        }
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return url;
}

// Function to copy to clipboard
void CopyToClipboard(const std::wstring& text) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        
        size_t size = (text.length() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
        
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), size);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        
        CloseClipboard();
    }
}

// Load settings
void LoadSettings() {
    std::ifstream file("settings.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("hotkey=") == 0) {
                std::string hotkey = line.substr(7);
                // Parse hotkey (e.g., "Ctrl+Shift+S")
                g_settings.hotkeyModifiers = 0;
                if (hotkey.find("Ctrl") != std::string::npos) g_settings.hotkeyModifiers |= MOD_CONTROL;
                if (hotkey.find("Shift") != std::string::npos) g_settings.hotkeyModifiers |= MOD_SHIFT;
                if (hotkey.find("Alt") != std::string::npos) g_settings.hotkeyModifiers |= MOD_ALT;
                
                size_t lastPlus = hotkey.find_last_of('+');
                if (lastPlus != std::string::npos && lastPlus + 1 < hotkey.length()) {
                    g_settings.hotkeyKey = toupper(hotkey[lastPlus + 1]);
                }
            }
        }
        file.close();
    }
}

// Save settings
void SaveSettings() {
    std::ofstream file("settings.txt");
    if (file.is_open()) {
        file << "# Nekoo Screenshot Settings\n";
        file << "# Hotkey format: Ctrl+Shift+S (or Ctrl+S, Alt+S, etc.)\n";
        file << "hotkey=";
        if (g_settings.hotkeyModifiers & MOD_CONTROL) file << "Ctrl+";
        if (g_settings.hotkeyModifiers & MOD_SHIFT) file << "Shift+";
        if (g_settings.hotkeyModifiers & MOD_ALT) file << "Alt+";
        file << (char)g_settings.hotkeyKey << "\n";
        file.close();
    }
}

// Handle screenshot
void HandleScreenshot() {
    std::wcout << L"\n📸 Capturing screenshot..." << std::endl;
    
    std::vector<BYTE> imageData = CaptureScreenshot();
    
    if (imageData.empty()) {
        std::wcerr << L"❌ Failed to capture" << std::endl;
        return;
    }
    
    std::wcout << L"⬆️  Uploading..." << std::endl;
    
    std::wstring url = UploadToNekoo(imageData);
    
    if (url.empty()) {
        std::wcerr << L"❌ Upload failed" << std::endl;
        return;
    }
    
    std::wcout << L"✅ " << url << std::endl;
    CopyToClipboard(url);
    std::wcout << L"📋 Copied to clipboard!" << std::endl;
}

int main() {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    std::wcout << L"Nekoo Screenshot Tool v1.0" << std::endl;
    std::wcout << L"============================" << std::endl;
    
    LoadSettings();
    SaveSettings(); // Create default settings if not exist
    
    std::wcout << L"Hotkey: ";
    if (g_settings.hotkeyModifiers & MOD_CONTROL) std::wcout << L"Ctrl+";
    if (g_settings.hotkeyModifiers & MOD_SHIFT) std::wcout << L"Shift+";
    if (g_settings.hotkeyModifiers & MOD_ALT) std::wcout << L"Alt+";
    std::wcout << (wchar_t)g_settings.hotkeyKey << std::endl;
    std::wcout << L"\nEdit settings.txt to change hotkey" << std::endl;
    std::wcout << L"Press Ctrl+C to exit\n" << std::endl;
    
    // Register hotkey
    if (!RegisterHotKey(NULL, HOTKEY_SCREENSHOT, g_settings.hotkeyModifiers, g_settings.hotkeyKey)) {
        std::wcerr << L"Failed to register hotkey!" << std::endl;
        GdiplusShutdown(gdiplusToken);
        return 1;
    }
    
    // Message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_SCREENSHOT) {
            HandleScreenshot();
        }
    }
    
    UnregisterHotKey(NULL, HOTKEY_SCREENSHOT);
    GdiplusShutdown(gdiplusToken);
    return 0;
}
