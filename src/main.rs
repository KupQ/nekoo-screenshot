use arboard::Clipboard;
use device_query::{DeviceQuery, DeviceState, Keycode};
use std::thread;
use std::time::Duration;

mod capture;
mod upload;

fn main() {
    println!("Nekoo Screenshot Tool v1.0.3");
    println!("Listening for hotkeys...");
    println!();
    println!("Hotkeys:");
    println!("  PrintScreen         - Capture fullscreen");
    println!("  Ctrl+Shift+S        - Capture region (currently fullscreen)");
    println!("  Ctrl+C (in console) - Exit");
    println!();

    let device_state = DeviceState::new();
    let mut last_keys = vec![];

    loop {
        let keys = device_state.get_keys();

        // Only trigger on new key press (not held)
        if keys != last_keys && !keys.is_empty() {
            // Check for PrintScreen
            if keys.contains(&Keycode::PrintScreen) {
                println!("📸 PrintScreen pressed - capturing fullscreen...");
                handle_capture();
            }
            // Check for Ctrl+Shift+S
            else if keys.contains(&Keycode::LControl) || keys.contains(&Keycode::RControl) {
                if (keys.contains(&Keycode::LShift) || keys.contains(&Keycode::RShift))
                    && keys.contains(&Keycode::S)
                {
                    println!("📸 Ctrl+Shift+S pressed - capturing region...");
                    handle_capture();
                }
            }
        }

        last_keys = keys;
        thread::sleep(Duration::from_millis(50));
    }
}

fn handle_capture() {
    thread::spawn(|| {
        // Capture screenshot
        let image_data = match capture::capture_fullscreen() {
            Ok(data) => data,
            Err(e) => {
                eprintln!("❌ Capture failed: {}", e);
                return;
            }
        };

        println!("✅ Screenshot captured ({} bytes)", image_data.len());
        println!("⬆️  Uploading to nekoo.ru...");

        match upload::upload_to_nekoo(&image_data) {
            Ok(url) => {
                println!("✅ Upload successful!");
                println!("🔗 {}", url);

                // Copy to clipboard
                if let Ok(mut clipboard) = Clipboard::new() {
                    if clipboard.set_text(&url).is_ok() {
                        println!("📋 Link copied to clipboard!");
                    }
                }
                println!();
            }
            Err(e) => {
                eprintln!("❌ Upload failed: {}", e);
                println!();
            }
        }
    });
}
