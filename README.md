# Nekoo Screenshot Tool

**Fast screenshot tool with hotkeys that uploads to nekoo.ru**

## Installation

Download the latest release:
- **Windows:** [nekoo-screenshot-windows.exe](https://github.com/KupQ/nekoo-screenshot/releases/latest)
- **macOS:** [nekoo-screenshot-macos](https://github.com/KupQ/nekoo-screenshot/releases/latest)

## Usage

Run the executable - it will stay in the background listening for hotkeys:

```bash
nekoo-screenshot
```

### Hotkeys

- **PrintScreen** - Capture fullscreen and upload
- **Ctrl+Shift+S** - Capture region and upload (currently fullscreen)

The link is automatically copied to your clipboard!

## Features

- ✅ Global hotkey support (PrintScreen, Ctrl+Shift+S)
- ✅ Instant upload to nekoo.ru
- ✅ Auto-copy link to clipboard
- ✅ Cross-platform (Windows & macOS)
- ✅ Lightweight background process

## Building from Source

```bash
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot
cargo build --release
```

Binary will be in `target/release/`

## License

MIT License - see [LICENSE](LICENSE)

---

Made with ❤️ for [nekoo.ru](https://nekoo.ru)
