use xcap::Monitor;
use std::io::Cursor;
use image::{ImageFormat, DynamicImage, ImageBuffer, Rgba};

pub fn capture_fullscreen() -> Result<Vec<u8>, String> {
    // Get primary monitor
    let monitors = Monitor::all().map_err(|e| format!("Failed to get monitors: {}", e))?;
    let monitor = monitors.first().ok_or("No monitor found")?;

    // Capture screenshot
    let image = monitor
        .capture_image()
        .map_err(|e| format!("Failed to capture screen: {}", e))?;

    // Convert to PNG bytes
    let mut bytes = Vec::new();
    let width = image.width();
    let height = image.height();
    let rgba_data = image.rgba();
    
    let img_buffer: ImageBuffer<Rgba<u8>, Vec<u8>> = ImageBuffer::from_raw(width, height, rgba_data.to_vec())
        .ok_or("Failed to create image buffer")?;
    
    let dynamic_image = DynamicImage::ImageRgba8(img_buffer);
    dynamic_image
        .write_to(&mut Cursor::new(&mut bytes), ImageFormat::Png)
        .map_err(|e| format!("Failed to encode image: {}", e))?;

    Ok(bytes)
}

pub fn capture_region() -> Result<Vec<u8>, String> {
    // For now, fallback to fullscreen
    println!("Region capture not yet implemented, using fullscreen");
    capture_fullscreen()
}
