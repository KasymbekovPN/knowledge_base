
macro_rules! make_builder_v1 {
    ($name:ident, $builder:ident { $f1:ident : $t1:ty, $f2:ident : $t2:ty }) => {
        struct $name {
            $f1: $t1,
            $f2: $t2,
        }

        #[derive(Default)]
        struct $builder {
            $f1: Option<$t1>,
            $f2: Option<$t2>,
        }

        impl $builder {
            fn $f1(mut self, value: $t1) -> Self {
                self.$f1 = Some(value);
                self
            }
            fn $f2(mut self, value: $t2) -> Self {
                self.$f2 = Some(value);
                self
            }
            fn build(self) -> $name {
                return $name {
                    $f1: self.$f1.expect("field not set"),
                    $f2: self.$f2.expect("field not set"),
                }
            }
        }

        impl $name {
            fn builder() -> $builder {
                $builder::default()
            }
        }
    };
}

make_builder_v1!(Person, PersonBuilder {name: String, age: u32});

fn main() {
    let p = Person::builder()
        .name("Pablo".to_string())
        .age(30)
        .build();
    println!("{} is {}", p.name, p.age)
}