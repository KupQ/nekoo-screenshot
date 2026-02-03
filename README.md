# Nekoo Screenshot Tool

**Windows screenshot tool with customizable hotkeys that uploads to nekoo.ru**

## Download

[Download nekoo-screenshot.exe](https://github.com/KupQ/nekoo-screenshot/releases/latest)

## Usage

1. Run `nekoo-screenshot.exe`
2. It runs in background listening for hotkey
3. Press **Ctrl+Shift+S** (default) to capture
4. Screenshot uploads and link copies to clipboard

## Customizing Hotkey

Edit `settings.txt` next to the .exe:

```
# Nekoo Screenshot Settings
# Hotkey format: Ctrl+Shift+S (or Ctrl+S, Alt+S, etc.)
hotkey=Ctrl+Shift+S
```

Change to any combination:
- `Ctrl+S`
- `Alt+S`
- `Ctrl+Alt+S`
- `Shift+F12`
- etc.

Restart the tool after changing settings.

## Features

- ✅ Global hotkey support (customizable)
- ✅ Runs in background
- ✅ Instant upload to nekoo.ru
- ✅ Auto-copy link to clipboard
- ✅ Native Windows (no dependencies)
- ✅ Settings file for configuration

## Building from Source

Requirements:
- Visual Studio 2022 or later
- CMake 3.15+

```bash
git clone https://github.com/KupQ/nekoo-screenshot.git
cd nekoo-screenshot
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Binary: `build/Release/nekoo-screenshot.exe`

## License

MIT License

---

Made for [nekoo.ru](https://nekoo.ru)
