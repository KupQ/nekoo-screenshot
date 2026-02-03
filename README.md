# Nekoo Screenshot Tool

**Fast screenshot tool that uploads to nekoo.ru**

## Installation

Download the latest release for your platform:
- **Windows:** [nekoo-screenshot-windows.exe](https://github.com/KupQ/nekoo-screenshot/releases/latest)
- **macOS:** [nekoo-screenshot-macos](https://github.com/KupQ/nekoo-screenshot/releases/latest)

## Usage

Simply run the executable:
```bash
nekoo-screenshot
```

It will:
1. Capture your screen
2. Upload to nekoo.ru
3. Copy the link to your clipboard
4. Print the link to console

That's it!

## Building from Source

```bash
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot
cargo build --release
```

Binary will be in `target/release/`

## Features

- ✅ Fast fullscreen capture
- ✅ Instant upload to nekoo.ru
- ✅ Auto-copy link to clipboard
- ✅ Cross-platform (Windows & macOS)
- ✅ No dependencies or installation required

## Tip

Create a keyboard shortcut to run this tool for quick screenshots!

**Windows:**
1. Right-click the .exe → Create shortcut
2. Right-click shortcut → Properties
3. Set "Shortcut key" to your preferred key (e.g., Ctrl+Alt+S)

**macOS:**
1. System Preferences → Keyboard → Shortcuts
2. App Shortcuts → Add (+)
3. Select the app and assign a shortcut

## License

MIT License - see [LICENSE](LICENSE)

---

Made with ❤️ for [nekoo.ru](https://nekoo.ru)
