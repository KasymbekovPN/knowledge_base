use anyhow::{Context, Result, bail};

fn read_timeout(path: &str) -> Result<u32> {
    let content = std::fs::read_to_string(path)
        .with_context(|| format!("Failed to read file {}", path))?;
    let timeout: u32 = content.trim().parse()
        .context("Timeout must be a number")?;
    if timeout > 3600 {
        bail!("timeout {timeout} more then 3600");
    }
    Ok(timeout)
}

fn main() -> Result<()> {
    match read_timeout("non_exist.txt") {
        Ok(t) => println!("timeout: {t}"),
        Err(e) => {
            println!("error: {e}");
            println!("causes chain:");
            for cause in e.chain().skip(1) {
                println!("  -> {cause}");
            }
        }
    }
    Ok(())
}