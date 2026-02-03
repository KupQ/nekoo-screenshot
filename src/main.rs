use arboard::Clipboard;
use std::env;

mod capture;
mod upload;

fn main() {
    println!("Nekoo Screenshot Tool v1.0.2");
    println!();

    let args: Vec<String> = env::args().collect();
    
    if args.len() > 1 && args[1] == "--help" {
        print_help();
        return;
    }

    println!("📸 Capturing screenshot...");

    // Capture fullscreen
    let image_data = match capture::capture_fullscreen() {
        Ok(data) => data,
        Err(e) => {
            eprintln!("❌ Capture failed: {}", e);
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
        }
        Err(e) => {
            eprintln!("❌ Upload failed: {}", e);
            std::process::exit(1);
        }
    }
}

fn print_help() {
    println!("Usage: nekoo-screenshot");
    println!();
    println!("Captures fullscreen screenshot and uploads to nekoo.ru");
    println!("The link is automatically copied to your clipboard.");
    println!();
    println!("Options:");
    println!("  --help    Show this help message");
}
