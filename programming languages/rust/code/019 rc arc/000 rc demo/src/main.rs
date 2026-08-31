
use std::rc::Rc;

struct Data { value: i32 }

impl Drop for Data {
    fn drop(&mut self) {
        println!("Dropping {}", self.value);
    }
}

fn main() {
    let a = Rc::new(Data { value: 42 });
    println!("count after creation: {}", Rc::strong_count(&a));

    // НЕ глубокая копия -- просто увеличивает счётчик
    let b = Rc::clone(&a);
    println!("count after clone in b: {}", Rc::strong_count(&a));

    {
        let c = Rc::clone(&a);
        println!("count after clone in c: {}", Rc::strong_count(&a));
        // Deref работает как у Box
        println!("c.value : {}", c.value);
    } // c выходит из scope -- счётчик уменьшается

    println!("count after scope : {}", Rc::strong_count(&a));
    println!("a.value = {}, b.value = {}", a.value, b.value);
}