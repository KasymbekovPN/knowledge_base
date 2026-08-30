
trait Container {
    type Item;
    fn get(&self, i: usize) -> Option<&Self::Item>;
}

struct MyVec(Vec<i32>);
impl Container for MyVec {
    type Item = i32;
    fn get(&self, i: usize) -> Option<&Self::Item> {
        self.0.get(i)
    }
}

fn main() {
    let v = MyVec(vec![1, 2, 3]);
    println!("second: {:?}", v.get(1));
}