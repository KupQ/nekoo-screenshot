#include "settings.h"
#include <windows.h>

#define REGISTRY_KEY L"Software\\Seko\\Screenshot"
#define AUTOSTART_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AUTOSTART_VALUE L"Seko"

AppSettings LoadSettings() {
    AppSettings settings;
    
    // Default values
    settings.fullscreenModifiers = MOD_CONTROL | MOD_SHIFT;
    settings.fullscreenKey = 'S';
    settings.regionModifiers = MOD_CONTROL | MOD_SHIFT;
    settings.regionKey = 'R';
    settings.autoStart = false;
    
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwValue;
        DWORD dwSize = sizeof(DWORD);
        
        if (RegQueryValueEx(hKey, L"FullscreenModifiers", NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
            settings.fullscreenModifiers = dwValue;
        }
        if (RegQueryValueEx(hKey, L"FullscreenKey", NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
            settings.fullscreenKey = dwValue;
        }
        if (RegQueryValueEx(hKey, L"RegionModifiers", NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
            settings.regionModifiers = dwValue;
        }
        if (RegQueryValueEx(hKey, L"RegionKey", NULL, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS) {
            settings.regionKey = dwValue;
        }
        
        RegCloseKey(hKey);
    }
    
    settings.autoStart = IsAutoStartEnabled();
    
    return settings;
}

void SaveSettings(const AppSettings& settings) {
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        DWORD dwValue;
        
        dwValue = settings.fullscreenModifiers;
        RegSetValueEx(hKey, L"FullscreenModifiers", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
        
        dwValue = settings.fullscreenKey;
        RegSetValueEx(hKey, L"FullscreenKey", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
        
        dwValue = settings.regionModifiers;
        RegSetValueEx(hKey, L"RegionModifiers", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
        
        dwValue = settings.regionKey;
        RegSetValueEx(hKey, L"RegionKey", 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
        
        RegCloseKey(hKey);
    }
    
    SetAutoStart(settings.autoStart);
}

bool IsAutoStartEnabled() {
    HKEY hKey;
    bool enabled = false;
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, AUTOSTART_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t path[MAX_PATH];
        DWORD dwSize = sizeof(path);
        
        if (RegQueryValueEx(hKey, AUTOSTART_VALUE, NULL, NULL, (LPBYTE)path, &dwSize) == ERROR_SUCCESS) {
            enabled = true;
        }
        
        RegCloseKey(hKey);
    }
    
    return enabled;
}

void SetAutoStart(bool enable) {
    HKEY hKey;
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, AUTOSTART_KEY, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        if (enable) {
            wchar_t exePath[MAX_PATH];
            GetModuleFileName(NULL, exePath, MAX_PATH);
            
            RegSetValueEx(hKey, AUTOSTART_VALUE, 0, REG_SZ, (LPBYTE)exePath, (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));
        } else {
            RegDeleteValue(hKey, AUTOSTART_VALUE);
        }
        
        RegCloseKey(hKey);
    }
}
