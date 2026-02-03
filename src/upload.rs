use reqwest::blocking::Client;
use reqwest::blocking::multipart;

pub fn upload_to_nekoo(image_data: &[u8]) -> Result<String, String> {
    let client = Client::new();

    // Create multipart form
    let part = multipart::Part::bytes(image_data.to_vec())
        .file_name("screenshot.png")
        .mime_str("image/png")
        .map_err(|e| format!("Failed to create multipart: {}", e))?;

    let form = multipart::Form::new().part("file", part);

    // Upload to nekoo.ru
    let response = client
        .post("https://nekoo.ru/upload")
        .multipart(form)
        .send()
        .map_err(|e| format!("Upload request failed: {}", e))?;

    if !response.status().is_success() {
        return Err(format!("Upload failed with status: {}", response.status()));
    }

    // Parse response
    let body = response
        .text()
        .map_err(|e| format!("Failed to read response: {}", e))?;

    // Extract URL from HTML response
    // Nekoo returns HTML with the URL in various places
    if let Some(start) = body.find("https://nekoo.ru/") {
        let url_part = &body[start..];
        // Find the end of the URL (usually a quote or whitespace)
        let end = url_part
            .find('"')
            .or_else(|| url_part.find('\''))
            .or_else(|| url_part.find('<'))
            .or_else(|| url_part.find(' '))
            .unwrap_or(url_part.len());
        
        let url = &url_part[..end];
        return Ok(url.to_string());
    }

    Err("Failed to extract URL from response".to_string())
}
