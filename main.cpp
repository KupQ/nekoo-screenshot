#include <windows.h>
#include <winhttp.h>
#include <gdiplus.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winhttp.lib")

using namespace Gdiplus;

std::vector<BYTE> CaptureScreen() {
    std::vector<BYTE> data;
    
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
    
    data.resize(size);
    LARGE_INTEGER pos;
    pos.QuadPart = 0;
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

std::wstring Upload(const std::vector<BYTE>& img) {
    HINTERNET hSession = WinHttpOpen(L"Nekoo/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return L"";
    
    HINTERNET hConnect = WinHttpConnect(hSession, L"nekoo.ru", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return L"";
    }
    
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/upload", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"";
    }
    
    std::ostringstream body;
    body << "--B\r\nContent-Disposition: form-data; name=\"file\"; filename=\"s.png\"\r\nContent-Type: image/png\r\n\r\n";
    body.write((const char*)img.data(), img.size());
    body << "\r\n--B--\r\n";
    std::string b = body.str();
    
    WinHttpAddRequestHeaders(hRequest, L"Content-Type: multipart/form-data; boundary=B", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)b.c_str(), (DWORD)b.length(), (DWORD)b.length(), 0);
    WinHttpReceiveResponse(hRequest, NULL);
    
    std::string resp;
    DWORD sz;
    while (WinHttpQueryDataAvailable(hRequest, &sz) && sz > 0) {
        std::vector<char> buf(sz + 1);
        DWORD read;
        WinHttpReadData(hRequest, buf.data(), sz, &read);
        buf[read] = '\0';
        resp += buf.data();
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    size_t p = resp.find("https://nekoo.ru/");
    if (p != std::string::npos) {
        size_t e = resp.find_first_of("\"'< ", p);
        std::string url = resp.substr(p, e - p);
        return std::wstring(url.begin(), url.end());
    }
    return L"";
}

void CopyClip(const std::wstring& txt) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        size_t size = (txt.length() + 1) * sizeof(wchar_t);
        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
        if (h) {
            void* mem = GlobalLock(h);
            memcpy(mem, txt.c_str(), size);
            GlobalUnlock(h);
            SetClipboardData(CF_UNICODETEXT, h);
        }
        CloseClipboard();
    }
}

void DoCapture() {
    std::wcout << L"\n📸 Capturing..." << std::endl;
    std::vector<BYTE> img = CaptureScreen();
    if (img.empty()) {
        std::wcerr << L"❌ Failed\n";
        return;
    }
    
    std::wcout << L"⬆️  Uploading..." << std::endl;
    std::wstring url = Upload(img);
    if (url.empty()) {
        std::wcerr << L"❌ Upload failed\n";
        return;
    }
    
    std::wcout << L"✅ " << url << std::endl;
    CopyClip(url);
    std::wcout << L"📋 Copied!\n";
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    
    GdiplusStartupInput gsi;
    ULONG_PTR token;
    GdiplusStartup(&token, &gsi, NULL);
    
    std::wcout << L"Nekoo Screenshot v2.0\n";
    std::wcout << L"Hotkey: Ctrl+Shift+S\n";
    std::wcout << L"Press Ctrl+C to exit\n\n";
    
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT, 'S')) {
        std::wcerr << L"Failed to register hotkey!\n";
        return 1;
    }
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            DoCapture();
        }
    }
    
    UnregisterHotKey(NULL, 1);
    GdiplusShutdown(token);
    return 0;
}
