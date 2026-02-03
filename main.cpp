#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
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

#define HOTKEY_SCREENSHOT 1

struct Settings {
    int hotkeyModifiers = MOD_CONTROL | MOD_SHIFT;
    int hotkeyKey = 'S';
};

Settings g_settings;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;
    
    GetImageEncoders(num, size, pImageCodecInfo);
    
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    
    free(pImageCodecInfo);
    return -1;
}

std::vector<BYTE> CaptureScreenshot() {
    std::vector<BYTE> imageData;
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);
    
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
    
    Bitmap* bitmap = new Bitmap(hBitmap, NULL);
    IStream* stream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &stream);
    
    CLSID pngClsid;
    GetEncoderClsid(L"image/png", &pngClsid);
    bitmap->Save(stream, &pngClsid, NULL);
    
    STATSTG statstg;
    stream->Stat(&statstg, STATFLAG_DEFAULT);
    ULONG size = (ULONG)statstg.cbSize.QuadPart;
    
    imageData.resize(size);
    LARGE_INTEGER li = {0};
    stream->Seek(li, STREAM_SEEK_SET, NULL);
    ULONG bytesRead = 0;
    stream->Read(imageData.data(), size, &bytesRead);
    stream->Release();
    
    delete bitmap;
    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return imageData;
}

std::wstring UploadToNekoo(const std::vector<BYTE>& imageData) {
    std::wstring url;
    
    HINTERNET hSession = WinHttpOpen(L"Nekoo/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    
    if (!hSession) return L"";
    
    HINTERNET hConnect = WinHttpConnect(hSession, L"nekoo.ru",
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
    
    std::string boundary = "----Boundary";
    std::ostringstream body;
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"file\"; filename=\"screenshot.png\"\r\n";
    body << "Content-Type: image/png\r\n\r\n";
    body.write((const char*)imageData.data(), imageData.size());
    body << "\r\n--" << boundary << "--\r\n";
    
    std::string bodyStr = body.str();
    std::wstring contentType = L"Content-Type: multipart/form-data; boundary=----Boundary";
    
    WinHttpAddRequestHeaders(hRequest, contentType.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
    
    BOOL result = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)bodyStr.c_str(), (DWORD)bodyStr.length(),
        (DWORD)bodyStr.length(), 0);
    
    if (result) {
        result = WinHttpReceiveResponse(hRequest, NULL);
        
        if (result) {
            DWORD dwSize = 0;
            std::string response;
            
            do {
                dwSize = 0;
                if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                    std::vector<char> buffer(dwSize + 1);
                    DWORD dwDownloaded = 0;
                    
                    if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                        buffer[dwDownloaded] = '\0';
                        response += buffer.data();
                    }
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

void CopyToClipboard(const std::wstring& text) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        
        size_t size = (text.length() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
        
        if (hMem) {
            void* pMem = GlobalLock(hMem);
            memcpy(pMem, text.c_str(), size);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
        
        CloseClipboard();
    }
}

void LoadSettings() {
    std::ifstream file("settings.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("hotkey=") == 0) {
                std::string hotkey = line.substr(7);
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

void SaveSettings() {
    std::ofstream file("settings.txt");
    if (file.is_open()) {
        file << "# Nekoo Screenshot Settings\n";
        file << "hotkey=";
        if (g_settings.hotkeyModifiers & MOD_CONTROL) file << "Ctrl+";
        if (g_settings.hotkeyModifiers & MOD_SHIFT) file << "Shift+";
        if (g_settings.hotkeyModifiers & MOD_ALT) file << "Alt+";
        file << (char)g_settings.hotkeyKey << "\n";
        file.close();
    }
}

void HandleScreenshot() {
    std::wcout << L"\n📸 Capturing..." << std::endl;
    
    std::vector<BYTE> imageData = CaptureScreenshot();
    
    if (imageData.empty()) {
        std::wcerr << L"❌ Failed" << std::endl;
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
    std::wcout << L"📋 Copied!" << std::endl;
}

int main() {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    std::wcout << L"Nekoo Screenshot v2.0" << std::endl;
    std::wcout << L"=====================" << std::endl;
    
    LoadSettings();
    SaveSettings();
    
    std::wcout << L"Hotkey: ";
    if (g_settings.hotkeyModifiers & MOD_CONTROL) std::wcout << L"Ctrl+";
    if (g_settings.hotkeyModifiers & MOD_SHIFT) std::wcout << L"Shift+";
    if (g_settings.hotkeyModifiers & MOD_ALT) std::wcout << L"Alt+";
    std::wcout << (wchar_t)g_settings.hotkeyKey << std::endl;
    std::wcout << L"\nEdit settings.txt to change" << std::endl;
    std::wcout << L"Press Ctrl+C to exit\n" << std::endl;
    
    if (!RegisterHotKey(NULL, HOTKEY_SCREENSHOT, g_settings.hotkeyModifiers, g_settings.hotkeyKey)) {
        std::wcerr << L"Failed to register hotkey!" << std::endl;
        GdiplusShutdown(gdiplusToken);
        return 1;
    }
    
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
