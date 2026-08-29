
mod solid {
    pub struct BankAccount {
        pub owner: String,
        pub balance: f64,
    }

    impl BankAccount {
        pub fn deposit(&mut self, amount: f64) { self.balance += amount; }
        pub fn withdraw(&mut self, amount: f64) -> Result<(), String> {
            if amount > self.balance {
                return Err(format!("Insufficient funds: {}", amount));
            }
            self.balance -= amount;
            Ok(())
        }
        pub fn display(&mut self) {
            println!("owner: {}, balance: {}", self.owner, self.balance)
        }
    }
}

mod separated {
    pub struct BankAccount {
        pub owner: String,
        pub balance: f64,
    }

    impl BankAccount {
        pub fn deposit(&mut self, amount: f64) { self.balance += amount; }
    }

    impl BankAccount {
        pub fn withdraw(&mut self, amount: f64) -> Result<(), String> {
            if amount > self.balance {
                return Err(format!("Insufficient funds: {}", amount));
            }
            self.balance -= amount;
            Ok(())
        }
        pub fn display(&mut self) {
            println!("owner: {}, balance: {}", self.owner, self.balance)
        }
    }
}


fn main() {
    let mut ba1 = separated::BankAccount {
        owner: String::from("o2"),
        balance: 0.0,
    };
    ba1.deposit(1000.0);
    ba1.withdraw(500.0).expect("!!!");
    ba1.display();

    let mut ba0 = solid::BankAccount {
        owner: String::from("o1"),
        balance: 0.0,
    };
    ba0.deposit(1000.0);
    ba0.withdraw(2000.0).expect("!!!");
}