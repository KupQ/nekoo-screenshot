// Platform-specific imports
#[cfg(any(target_os = "windows", target_os = "macos"))]
use arboard::Clipboard;
#[cfg(any(target_os = "windows", target_os = "macos"))]
use global_hotkey::{GlobalHotKeyEvent, GlobalHotKeyManager, hotkey::{Code, HotKey, Modifiers}};
#[cfg(any(target_os = "windows", target_os = "macos"))]
use notify_rust::Notification;
#[cfg(any(target_os = "windows", target_os = "macos"))]
use std::thread;
#[cfg(any(target_os = "windows", target_os = "macos"))]
use tray_icon::{
    menu::{Menu, MenuEvent, MenuItem},
    TrayIconBuilder,
};
#[cfg(any(target_os = "windows", target_os = "macos"))]
use winit::event_loop::{ControlFlow, EventLoopBuilder};

mod capture;
mod upload;
mod settings;

#[cfg(not(any(target_os = "windows", target_os = "macos")))]
fn main() {
    eprintln!("This application is only supported on Windows and macOS");
    eprintln!("Please build on the target platform");
    std::process::exit(1);
}

#[cfg(any(target_os = "windows", target_os = "macos"))]
fn main() {
    println!("Nekoo Screenshot Tool v1.0.0");
    println!("Starting...");

    // Create event loop
    let event_loop = EventLoopBuilder::new().build().expect("Failed to create event loop");

    // Initialize hotkey manager
    let hotkey_manager = GlobalHotKeyManager::new().expect("Failed to initialize hotkey manager");

    // Register PrtSc for fullscreen
    let fullscreen_hotkey = HotKey::new(None, Code::PrintScreen);
    hotkey_manager
        .register(fullscreen_hotkey)
        .expect("Failed to register fullscreen hotkey");

    // Register Ctrl+Shift+S for region
    let region_hotkey = HotKey::new(Some(Modifiers::CONTROL | Modifiers::SHIFT), Code::KeyS);
    hotkey_manager
        .register(region_hotkey)
        .expect("Failed to register region hotkey");

    println!("Hotkeys registered:");
    println!("  PrtSc - Fullscreen capture");
    println!("  Ctrl+Shift+S - Region capture");

    // Create system tray
    let tray_menu = Menu::new();
    let capture_fullscreen = MenuItem::new("Capture Fullscreen", true, None);
    let capture_region = MenuItem::new("Capture Region", true, None);
    let quit = MenuItem::new("Exit", true, None);

    tray_menu.append(&capture_fullscreen).unwrap();
    tray_menu.append(&capture_region).unwrap();
    tray_menu.append(&quit).unwrap();

    let _tray_icon = TrayIconBuilder::new()
        .with_menu(Box::new(tray_menu))
        .with_tooltip("Nekoo Screenshot")
        .build()
        .expect("Failed to create tray icon");

    println!("System tray initialized");

    // Channel for hotkey events
    let hotkey_receiver = GlobalHotKeyEvent::receiver();
    let menu_receiver = MenuEvent::receiver();

    // Run event loop
    event_loop.run(move |_event, _target| {
        _target.set_control_flow(ControlFlow::Wait);

        // Check for hotkey events
        if let Ok(event) = hotkey_receiver.try_recv() {
            if event.id == fullscreen_hotkey.id() {
                println!("Fullscreen hotkey pressed");
                handle_capture(CaptureMode::Fullscreen);
            } else if event.id == region_hotkey.id() {
                println!("Region hotkey pressed");
                handle_capture(CaptureMode::Region);
            }
        }

        // Check for menu events
        if let Ok(event) = menu_receiver.try_recv() {
            if event.id == capture_fullscreen.id() {
                handle_capture(CaptureMode::Fullscreen);
            } else if event.id == capture_region.id() {
                handle_capture(CaptureMode::Region);
            } else if event.id == quit.id() {
                _target.exit();
            }
        }
    }).expect("Event loop failed");
}

#[cfg(any(target_os = "windows", target_os = "macos"))]
#[derive(Clone, Copy)]
enum CaptureMode {
    Fullscreen,
    Region,
}

#[cfg(any(target_os = "windows", target_os = "macos"))]
fn handle_capture(mode: CaptureMode) {
    thread::spawn(move || {
        // Show notification
        let _ = Notification::new()
            .summary("Nekoo Screenshot")
            .body("Capturing screenshot...")
            .show();

        // Capture screenshot
        let image_data = match mode {
            CaptureMode::Fullscreen => capture::capture_fullscreen(),
            CaptureMode::Region => capture::capture_region(),
        };

        let image_data = match image_data {
            Ok(data) => data,
            Err(e) => {
                eprintln!("Capture failed: {}", e);
                let _ = Notification::new()
                    .summary("Nekoo Screenshot")
                    .body(&format!("Capture failed: {}", e))
                    .show();
                return;
            }
        };

        // Upload to nekoo.ru
        let _ = Notification::new()
            .summary("Nekoo Screenshot")
            .body("Uploading to nekoo.ru...")
            .show();

        match upload::upload_to_nekoo(&image_data) {
            Ok(url) => {
                println!("Upload successful: {}", url);

                // Copy to clipboard
                if let Ok(mut clipboard) = Clipboard::new() {
                    let _ = clipboard.set_text(&url);
                }

                // Show success notification
                let _ = Notification::new()
                    .summary("Nekoo Screenshot")
                    .body(&format!("Link copied!\n{}", url))
                    .show();
            }
            Err(e) => {
                eprintln!("Upload failed: {}", e);
                let _ = Notification::new()
                    .summary("Nekoo Screenshot")
                    .body(&format!("Upload failed: {}", e))
                    .show();
            }
        }
    });
}
