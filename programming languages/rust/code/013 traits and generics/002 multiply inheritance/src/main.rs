
trait Drawable {
    fn draw(&self);
    fn info(&self);
}

trait Serializable {
    fn serialize(&self) -> String;
    fn info(&self);
}

struct Widget;

impl Drawable for Widget {
    fn draw(&self) { println!("Drawable draw"); }
    fn info(&self) { println!("Drawable info"); }
}

impl Serializable for Widget {
    fn serialize(&self) -> String { "Serializable".to_string() }
    fn info(&self) { println!("Serializable info"); }
}

fn process<T: Drawable + Serializable>(item: &T) {
    item.draw();
    Drawable::info(item);
    println!("{}", item.serialize());
}

fn main() {
    let w = Widget {};
    process(&w);
}