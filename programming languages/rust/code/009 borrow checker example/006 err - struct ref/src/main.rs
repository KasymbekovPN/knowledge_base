
mod test0 {
    // struct Parser {
    //     // ОШИБКА: missing lifetime specifier
    //     input: &str,
    // }
}

mod test1 {
    struct Parser<'a> {
        input: &'a str,
    }

    impl<'a> Parser<'a> {
        fn new(input: &'a str) -> Self {
            Parser { input }
        }
    }
}

mod test2 {
    struct Parser {
        // владеет данными сам — никаких lifetime-параметров вообще
        input: String,
    }

    impl Parser {
        fn new (input: &str) ->Self {
            Parser { input: input.to_string() }
        }
    }
}

fn main() {
    
}