
struct Container {
    items: Vec<i32>,
}

impl Container {
    fn first_mut(&mut self) -> &mut i32 {
        &mut self.items[0]
    }

    fn broken(&mut self) {
        let first = self.first_mut();
        // ОШИБКА: cannot borrow `self.items` as mutable more than once
        // self.items.push(99);
        println!("{:?}", first);
    }

    fn non_broken(&mut self) {
        {
            let first = self.first_mut();
            println!("{:?}", first);
        }
        self.items.push(99);
    }
}

fn main() {
    let mut c = Container{items: vec![1, 2, 3]};
    c.non_broken();
}
