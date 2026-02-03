use arboard::Clipboard;
use rdev::{listen, Event, EventType, Key};
use std::sync::mpsc::{channel, Sender};
use std::thread;

mod capture;
mod upload;
mod settings;

fn main() {
    println!("Nekoo Screenshot Tool v1.0.0");
    println!("Starting...");
    println!();
    println!("Hotkeys:");
    println!("  PrintScreen - Capture fullscreen and upload");
    println!("  Ctrl+Shift+S - Capture region (currently fullscreen)");
    println!();
    println!("Press Ctrl+C to exit");
    println!();

    // Create channel for hotkey events
    let (tx, rx) = channel();

    // Spawn hotkey listener thread
    thread::spawn(move || {
        listen_for_hotkeys(tx);
    });

    // Main event loop
    loop {
        if let Ok(capture_mode) = rx.recv() {
            handle_capture(capture_mode);
        }
    }
}

#[derive(Clone, Copy, Debug)]
enum CaptureMode {
    Fullscreen,
    Region,
}

fn listen_for_hotkeys(tx: Sender<CaptureMode>) {
    let mut ctrl_pressed = false;
    let mut shift_pressed = false;

    let callback = move |event: Event| {
        match event.event_type {
            EventType::KeyPress(key) => {
                match key {
                    Key::ControlLeft | Key::ControlRight => ctrl_pressed = true,
                    Key::ShiftLeft | Key::ShiftRight => shift_pressed = true,
                    Key::PrintScreen => {
                        let _ = tx.send(CaptureMode::Fullscreen);
                    }
                    Key::KeyS if ctrl_pressed && shift_pressed => {
                        let _ = tx.send(CaptureMode::Region);
                    }
                    _ => {}
                }
            }
            EventType::KeyRelease(key) => {
                match key {
                    Key::ControlLeft | Key::ControlRight => ctrl_pressed = false,
                    Key::ShiftLeft | Key::ShiftRight => shift_pressed = false,
                    _ => {}
                }
            }
            _ => {}
        }
    };

    if let Err(error) = listen(callback) {
        eprintln!("Error listening for hotkeys: {:?}", error);
    }
}

fn handle_capture(mode: CaptureMode) {
    thread::spawn(move || {
        println!("📸 Capturing {:?}...", mode);

        // Capture screenshot
        let image_data = match mode {
            CaptureMode::Fullscreen => capture::capture_fullscreen(),
            CaptureMode::Region => capture::capture_region(),
        };

        let image_data = match image_data {
            Ok(data) => data,
            Err(e) => {
                eprintln!("❌ Capture failed: {}", e);
                return;
            }
        };

        println!("⬆️  Uploading to nekoo.ru...");

        match upload::upload_to_nekoo(&image_data) {
            Ok(url) => {
                println!("✅ Upload successful!");
                println!("🔗 {}", url);

                // Copy to clipboard
                if let Ok(mut clipboard) = Clipboard::new() {
                    let _ = clipboard.set_text(&url);
                    println!("📋 Link copied to clipboard!");
                } else {
                    println!("⚠️  Could not copy to clipboard");
                }
            }
            Err(e) => {
                eprintln!("❌ Upload failed: {}", e);
            }
        }
        println!();
    });
}
