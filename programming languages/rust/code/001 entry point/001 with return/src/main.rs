use std::fs::File;

fn main() -> Result<(), std::io::Error> {
    // если ошибка — main вернёт Err, процесс завершится с ненулевым кодом
    let _ = File::open("non_exist.toml")?;
    Ok(())
}