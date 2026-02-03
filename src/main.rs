use arboard::Clipboard;

mod capture;
mod upload;

fn main() {
    println!("Nekoo Screenshot Tool v1.0.4");
    println!();
    println!("📸 Capturing screenshot...");

    // Capture fullscreen
    let image_data = match capture::capture_fullscreen() {
        Ok(data) => data,
        Err(e) => {
            eprintln!("❌ Capture failed: {}", e);
            eprintln!();
            eprintln!("Press Enter to exit...");
            let mut input = String::new();
            let _ = std::io::stdin().read_line(&mut input);
            std::process::exit(1);
        }
    };

    println!("✅ Screenshot captured ({} bytes)", image_data.len());
    println!("⬆️  Uploading to nekoo.ru...");

    match upload::upload_to_nekoo(&image_data) {
        Ok(url) => {
            println!("✅ Upload successful!");
            println!();
            println!("🔗 {}", url);
            println!();

            // Copy to clipboard
            if let Ok(mut clipboard) = Clipboard::new() {
                if clipboard.set_text(&url).is_ok() {
                    println!("📋 Link copied to clipboard!");
                }
            }
            
            println!();
            println!("Press Enter to exit...");
            let mut input = String::new();
            let _ = std::io::stdin().read_line(&mut input);
        }
        Err(e) => {
            eprintln!("❌ Upload failed: {}", e);
            eprintln!();
            eprintln!("Press Enter to exit...");
            let mut input = String::new();
            let _ = std::io::stdin().read_line(&mut input);
            std::process::exit(1);
        }
    }
}
