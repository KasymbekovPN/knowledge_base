use domain::Product;
use std::collections::HashMap;

#[derive(Default)]
pub struct InMemoryStore {
    products: HashMap<String, Product>,
}

impl InMemoryStore {
    pub fn new() -> Self { Self::default() }
    pub fn save(&mut self, product: Product) {
        self.products.insert(product.name.clone(), product);
    }
    pub fn get(&self, name: &str) -> Option<&Product> {
        self.products.get(name)
    }
    pub fn len(&self) -> usize { self.products.len() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn save_and_get() {
        let mut store = InMemoryStore::new();
        let p = Product::new("book", 42).unwrap();
        store.save(p.clone());
        assert_eq!(store.products.get("book"), Some(&p));
    }
}
