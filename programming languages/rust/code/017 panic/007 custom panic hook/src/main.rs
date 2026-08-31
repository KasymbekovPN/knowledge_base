use std::panic;

fn main() {
    panic::set_hook(Box::new(|info| {
        let msg = info
            .payload()
            .downcast_ref::<&str>()
            .map(|s| s.to_string())
            .or_else(|| info.payload().downcast_ref::<String>().cloned())
            .unwrap_or_else(|| "Unknown".to_string());

        if let Some(loc) = info.location() {
            eprintln!("[CUSTOM] panic in {}:{}: {msg}", loc.file(), loc.line());
        }
    }));


    let _r = panic::catch_unwind(move || {
        panic!("with catch_unwind");
    });

    panic!("without catch_unwind");
}