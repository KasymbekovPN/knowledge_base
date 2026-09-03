use std::rc::Rc;
use std::cell::RefCell;

struct Account {
    balance: RefCell<f64>,
}

fn main() {
    let account = Rc::new(Account { balance: RefCell::new(100.0) });

    let a1 = Rc::clone(&account);
    let a2 = Rc::clone(&account);

    *a1.balance.borrow_mut() += 10.0;
    *a2.balance.borrow_mut() -= 50.0;

    println!("total balance: {}", account.balance.borrow());
    println!("strong_count: {}", Rc::strong_count(&account));
}