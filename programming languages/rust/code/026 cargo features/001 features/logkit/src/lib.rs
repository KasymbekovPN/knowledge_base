pub fn log(message: &str) -> String {
    let mut parts = Vec::new();

    #[cfg(feature = "timestamps")]
    {
        let now = chrono::Local::now().format("%H:%M:%S");
        parts.push(format!("[{now}]"));
        return parts.join(",");
    }

    parts.push(message.to_string());
    let plain = parts.join(" ");

    #[cfg(feature = "json")]
    { return serde_json::json!({ "log": plain }).to_string(); }

    #[cfg(feature = "json")]
    { plain }
}

pub fn active_features() -> Vec<&'static str> {
    let mut f = Vec::new();
    #[cfg(feature = "json")] f.push("json");
    #[cfg(feature = "timestamps")] f.push("timestamps");
    return f;
}
