use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

#[derive(Serialize, Deserialize, Clone)]
pub struct Settings {
    pub upload_endpoint: String,
    pub notifications_enabled: bool,
    pub auto_start: bool,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            upload_endpoint: "https://nekoo.ru/upload".to_string(),
            notifications_enabled: true,
            auto_start: false,
        }
    }
}

impl Settings {
    pub fn load() -> Self {
        let path = Self::settings_path();
        if let Ok(content) = fs::read_to_string(&path) {
            if let Ok(settings) = serde_json::from_str(&content) {
                return settings;
            }
        }
        Self::default()
    }

    pub fn save(&self) -> Result<(), String> {
        let path = Self::settings_path();
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent).map_err(|e| format!("Failed to create config dir: {}", e))?;
        }
        let content = serde_json::to_string_pretty(self)
            .map_err(|e| format!("Failed to serialize settings: {}", e))?;
        fs::write(&path, content).map_err(|e| format!("Failed to write settings: {}", e))?;
        Ok(())
    }

    fn settings_path() -> PathBuf {
        let mut path = dirs::config_dir().unwrap_or_else(|| PathBuf::from("."));
        path.push("nekoo-screenshot");
        path.push("settings.json");
        path
    }
}
