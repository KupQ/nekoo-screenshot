# Nekoo Screenshot Tool

**Fast screenshot tool that uploads to nekoo.ru**

## Installation

Download the latest release:
- **Windows:** [nekoo-screenshot-windows.exe](https://github.com/KupQ/nekoo-screenshot/releases/latest)
- **macOS:** [nekoo-screenshot-macos](https://github.com/KupQ/nekoo-screenshot/releases/latest)

## Usage

Run the executable to capture and upload:
```bash
nekoo-screenshot
```

It will:
1. Capture your fullscreen
2. Upload to nekoo.ru
3. Copy link to clipboard
4. Show the link

## Setting Up Hotkey

### Windows
1. Right-click `nekoo-screenshot.exe` → **Create shortcut**
2. Right-click the shortcut → **Properties**
3. In "Shortcut key" field, press **PrintScreen** (or any key combo)
4. Click **OK**
5. Move shortcut to Desktop or Start Menu

Now pressing PrintScreen will run the tool!

### macOS
1. Open **System Preferences** → **Keyboard** → **Shortcuts**
2. Click **App Shortcuts** → **+** button
3. Choose **All Applications**
4. Menu Title: (leave blank)
5. Keyboard Shortcut: Press your desired key (e.g., Cmd+Shift+3)
6. Click **Add**

Or use Automator to create a Quick Action bound to a hotkey.

## Features

- ✅ Fast fullscreen capture
- ✅ Instant upload to nekoo.ru
- ✅ Auto-copy link to clipboard
- ✅ Cross-platform (Windows & macOS)
- ✅ Minimal dependencies

## Building from Source

```bash
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot
cargo build --release
```

## License

MIT License - see [LICENSE](LICENSE)

---

Made with ❤️ for [nekoo.ru](https://nekoo.ru)
