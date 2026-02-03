use scrap::{Capturer, Display};
use std::io::Cursor;
use image::{ImageFormat, DynamicImage, ImageBuffer, Rgba};
use std::thread;
use std::time::Duration;

pub fn capture_fullscreen() -> Result<Vec<u8>, String> {
    // Get primary display
    let display = Display::primary().map_err(|e| format!("Failed to get display: {}", e))?;
    
    // Create capturer
    let mut capturer = Capturer::new(display).map_err(|e| format!("Failed to create capturer: {}", e))?;
    
    // Wait a bit for the capturer to initialize
    thread::sleep(Duration::from_millis(100));
    
    // Capture frame
    let frame = loop {
        match capturer.frame() {
            Ok(frame) => break frame,
            Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                thread::sleep(Duration::from_millis(10));
                continue;
            }
            Err(e) => return Err(format!("Failed to capture frame: {}", e)),
        }
    };
    
    let width = capturer.width();
    let height = capturer.height();
    
    // Convert BGRA to RGBA
    let mut rgba_data = Vec::with_capacity(frame.len());
    for chunk in frame.chunks(4) {
        rgba_data.push(chunk[2]); // R
        rgba_data.push(chunk[1]); // G
        rgba_data.push(chunk[0]); // B
        rgba_data.push(chunk[3]); // A
    }
    
    // Create image
    let img_buffer: ImageBuffer<Rgba<u8>, Vec<u8>> = ImageBuffer::from_raw(width as u32, height as u32, rgba_data)
        .ok_or("Failed to create image buffer")?;
    
    let dynamic_image = DynamicImage::ImageRgba8(img_buffer);
    
    // Convert to PNG bytes
    let mut bytes = Vec::new();
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
