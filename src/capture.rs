use screenshots::Screen;
use std::io::Cursor;
use image::{ImageFormat, DynamicImage};

pub fn capture_fullscreen() -> Result<Vec<u8>, String> {
    // Get primary screen
    let screens = Screen::all().map_err(|e| format!("Failed to get screens: {}", e))?;
    let screen = screens.first().ok_or("No screen found")?;

    // Capture screenshot
    let image = screen
        .capture()
        .map_err(|e| format!("Failed to capture screen: {}", e))?;

    // Convert to PNG bytes
    let mut bytes = Vec::new();
    let dynamic_image = DynamicImage::ImageRgba8(image);
    dynamic_image
        .write_to(&mut Cursor::new(&mut bytes), ImageFormat::Png)
        .map_err(|e| format!("Failed to encode image: {}", e))?;

    Ok(bytes)
}

pub fn capture_region() -> Result<Vec<u8>, String> {
    // For now, fallback to fullscreen
    // TODO: Implement region selection overlay
    println!("Region capture not yet implemented, using fullscreen");
    capture_fullscreen()
}
