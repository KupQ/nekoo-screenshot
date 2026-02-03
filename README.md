# Nekoo Screenshot Tool

<p align="center">
  <img src="https://nekoo.ru/logo.png" alt="Nekoo Logo" width="120">
</p>

<p align="center">
  <strong>Fast, lightweight screenshot tool that uploads to nekoo.ru</strong>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage">Usage</a> •
  <a href="#building">Building</a> •
  <a href="#license">License</a>
</p>

---

## Features

- 📸 **Fullscreen & Region Capture** - Capture entire screen or select custom area
- ⚡ **Instant Upload** - Automatically uploads to nekoo.ru
- 📋 **Auto-Copy Link** - Link copied to clipboard immediately
- ⌨️ **Customizable Hotkeys** - Set your own keyboard shortcuts
- 🖥️ **Cross-Platform** - Works on Windows & macOS
- 🎨 **Minimal System Tray** - Lightweight background app
- 🔔 **Notifications** - Desktop notifications for upload status

## Installation

### Windows
1. Download `nekoo-screenshot-windows.msi` from [Releases](https://github.com/KupQ/nekoo-screenshot/releases)
2. Run the installer
3. The app will start automatically and appear in your system tray

### macOS
1. Download `nekoo-screenshot-macos.dmg` from [Releases](https://github.com/KupQ/nekoo-screenshot/releases)
2. Open the DMG and drag to Applications
3. Launch from Applications folder

## Usage

### Default Hotkeys
- **PrtSc** (Print Screen) - Capture fullscreen
- **Ctrl+Shift+S** - Capture custom region

### Workflow
1. Press hotkey to capture
2. For region capture: Click and drag to select area
3. Screenshot uploads automatically
4. Link copied to clipboard
5. Notification shows success with URL

### Settings
Right-click the system tray icon and select **Settings** to:
- Change hotkeys
- Enable/disable notifications
- View upload history
- Configure auto-start

## Building from Source

### Prerequisites
- Rust 1.70+ ([Install Rust](https://rustup.rs/))
- Git

### Build Steps
```bash
# Clone repository
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot

# Build release binary
cargo build --release

# Binary location
# Windows: target/release/nekoo-screenshot.exe
# macOS: target/release/nekoo-screenshot
```

### Platform-Specific Requirements

**Windows:**
- Visual Studio Build Tools or MSVC

**macOS:**
- Xcode Command Line Tools
```bash
xcode-select --install
```

## Configuration

Settings are stored in:
- **Windows:** `%APPDATA%\nekoo-screenshot\settings.json`
- **macOS:** `~/Library/Application Support/nekoo-screenshot/settings.json`
- **Linux:** `~/.config/nekoo-screenshot/settings.json`

Example configuration:
```json
{
  "hotkeys": {
    "fullscreen": "PrintScreen",
    "region": "Ctrl+Shift+S"
  },
  "upload_endpoint": "https://nekoo.ru/upload",
  "notifications_enabled": true,
  "auto_start": true
}
```

## How It Works

1. **Capture:** Uses platform-specific APIs to capture screen
2. **Upload:** Sends PNG to nekoo.ru via multipart form POST
3. **Response:** Parses response to extract file URL
4. **Clipboard:** Copies URL using cross-platform clipboard library
5. **Notify:** Shows desktop notification with result

## Troubleshooting

### Windows: Hotkey not working
- Check if another app is using the same hotkey
- Try running as administrator
- Change hotkey in settings

### macOS: Permission denied
- Grant Screen Recording permission in System Preferences > Security & Privacy
- Grant Accessibility permission for hotkeys

### Upload fails
- Check internet connection
- Verify nekoo.ru is accessible
- Check firewall settings

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

MIT License - see [LICENSE](LICENSE) file for details

## Acknowledgments

- Built with [Rust](https://www.rust-lang.org/)
- Uses [screenshots](https://github.com/nashaofu/screenshots-rs) for capture
- Uses [global-hotkey](https://github.com/tauri-apps/global-hotkey) for hotkeys
- Powered by [nekoo.ru](https://nekoo.ru)

---

<p align="center">
  Made with ❤️ for the Nekoo community
</p>
