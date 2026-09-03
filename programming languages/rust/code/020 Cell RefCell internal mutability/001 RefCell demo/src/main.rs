use std::cell::RefCell;

#[derive(Default)]
struct Log {
    messages: RefCell<Vec<String>>,
}

// тоже &self, не &mut self
impl Log {
    fn add(&self, message: &str) {
        self.messages.borrow_mut().push(message.to_string());
    }

    fn print_all(&self) {
        for m in self.messages.borrow().iter() {
            println!("{}", m);
        }
    }
}

fn main() {
    let log = Log::default();
    log.add("first message");
    log.add("second message");

    log.print_all();
}