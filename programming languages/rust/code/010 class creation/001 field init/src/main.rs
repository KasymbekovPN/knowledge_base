
pub struct Config1 {
    timeout: i32,
    verbose: bool,
}

impl Default for Config1 {
    fn default() -> Self {
        Config1 { timeout: 30, verbose: false}
    }
}

impl Config1 {
    pub fn display(&self) {
        println!("{}, {}", self.timeout, self.verbose);
    }
}

#[derive(Default)]
pub struct Config2 {
    timeout: i32,
    verbose: bool,
}

impl Config2 {
    pub fn display(&self) {
        println!("{}, {}", self.timeout, self.verbose);
    }
}

fn main() {
    let c00 = Config1::default();
    c00.display();

    let c01 = Config1 { timeout: 60, verbose: true };
    c01.display();

    let c10 = Config2::default();
    c10.display();

    let c11 = Config2 { timeout: 60, ..Default::default() };
    c11.display();
}