use domain::Product;
use storage::InMemoryStore;

fn main() {
    let mut store = InMemoryStore::new();

    match Product::new("keyboard", 45000) {
        Ok(p) => store.save(p),
        Err(e) => eprintln!("Error: {e}"),
    }
    match Product::new("out-of-date", 0) {
        Ok(p) => store.save(p),
        Err(e) => eprintln!("Error: {e}"),
    }

    if let Some(p) = store.get("keyboard") {
        println!("found: {} in {} cents", p.name, p.price_cents)
    }
    println!("total goods count: {}", store.len())
}
