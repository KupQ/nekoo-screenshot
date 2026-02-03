#include <windows.h>
#include <winhttp.h>
#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "winhttp.lib")

std::wstring UploadToNekoo(const std::vector<BYTE>& imageData) {
    HINTERNET hSession = WinHttpOpen(L"Nekoo/3.0",
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
    
    // Create multipart form data
    std::ostringstream body;
    body << "--BOUNDARY\r\n";
    body << "Content-Disposition: form-data; name=\"file\"; filename=\"screenshot.png\"\r\n";
    body << "Content-Type: image/png\r\n\r\n";
    body.write((const char*)imageData.data(), imageData.size());
    body << "\r\n--BOUNDARY--\r\n";
    
    std::string bodyStr = body.str();
    
    WinHttpAddRequestHeaders(hRequest,
        L"Content-Type: multipart/form-data; boundary=BOUNDARY",
        (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    
    BOOL result = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)bodyStr.c_str(), (DWORD)bodyStr.length(),
        (DWORD)bodyStr.length(), 0);
    
    std::wstring url;
    
    if (result) {
        result = WinHttpReceiveResponse(hRequest, NULL);
        
        if (result) {
            std::string response;
            DWORD dwSize;
            
            while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1);
                DWORD dwDownloaded;
                
                if (WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) {
                    buffer[dwDownloaded] = '\0';
                    response += buffer.data();
                }
            }
            
            // Extract URL from response
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
