# Nekoo Screenshot Tool

**Professional Windows screenshot tool with GUI, system tray, and auto-upload to nekoo.ru**

![Version](https://img.shields.io/badge/version-3.0.0-purple)
![Platform](https://img.shields.io/badge/platform-Windows-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

- 🎯 **System Tray Application** - Runs in background, no console window
- ⌨️ **Global Hotkeys**
  - `Ctrl+Shift+S` - Capture fullscreen
  - `Ctrl+Shift+R` - Capture region (click & drag)
- 🖼️ **Region Selection** - Visual overlay with purple border and dimensions
- ☁️ **Auto-Upload** - Instantly uploads to nekoo.ru
- 📋 **Toast Notifications** - Shows link with copy button
- 🎨 **Nekoo Design** - Dark theme with purple accents
- 🚀 **Auto-Start** - Optional Windows startup integration

## 📥 Download

**[Download Latest Release](https://github.com/KupQ/nekoo-screenshot/releases/latest)**

### Installation Options

1. **Installer (Recommended)** - `nekoo-screenshot-setup.exe`
   - Automatic installation
   - Start with Windows option
   - Desktop shortcut
   - Easy uninstall

2. **Portable** - `nekoo-screenshot.exe`
   - No installation required
   - Run directly
   - Manual startup configuration

## 🚀 Usage

### Quick Start

1. Run the application (system tray icon appears)
2. Press `Ctrl+Shift+S` for fullscreen or `Ctrl+Shift+R` for region
3. Screenshot uploads automatically
4. Click "Copy Link" in the notification
5. Paste anywhere!

### Tray Menu

Right-click the system tray icon:
- **Capture Fullscreen** - Take fullscreen screenshot
- **Capture Region** - Select area to capture
- **Settings** - Configure hotkeys (coming soon)
- **Exit** - Close application

### Region Selection

1. Press `Ctrl+Shift+R` or select from menu
2. Click and drag to select area
3. Release to capture
4. Press `ESC` to cancel

## 🛠️ Building from Source

### Requirements

- Visual Studio 2022 or later
- CMake 3.15+
- Windows 10/11

### Build Steps

```bash
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Executable: `build/Release/nekoo-screenshot.exe`

### Create Installer

Requires [Inno Setup 6](https://jrsoftware.org/isinfo.php)

```bash
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\setup.iss
```

Installer: `installer/nekoo-screenshot-setup.exe`

## 🎨 Screenshots

### System Tray
Application runs silently in the system tray with quick access menu.

### Region Selection
Visual overlay with purple border shows selected area and dimensions in real-time.

### Toast Notification
Dark-themed notification displays the uploaded link with a copy button.

## ⚙️ Configuration

Settings are stored in: `%APPDATA%\Nekoo\screenshot-config.json`

```json
{
  "hotkey": {
    "fullscreen": {
      "modifiers": ["Ctrl", "Shift"],
      "key": "S"
    },
    "region": {
      "modifiers": ["Ctrl", "Shift"],
      "key": "R"
    }
  },
  "upload_url": "https://nekoo.ru/upload",
  "auto_start": true
}
```

## 🔧 Troubleshooting

### Hotkeys not working
- Check if another application is using the same hotkey
- Try restarting the application
- Check Windows hotkey conflicts

### Upload fails
- Verify internet connection
- Check if nekoo.ru is accessible
- Try again after a few seconds

### Application won't start
- Check if it's already running (system tray)
- Run as administrator
- Reinstall using the installer

## 📝 License

MIT License - see [LICENSE](LICENSE) file

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 🔗 Links

- **Website**: [nekoo.ru](https://nekoo.ru)
- **Issues**: [GitHub Issues](https://github.com/KupQ/nekoo-screenshot/issues)
- **Releases**: [GitHub Releases](https://github.com/KupQ/nekoo-screenshot/releases)

---

Made with 💜 for [nekoo.ru](https://nekoo.ru)
