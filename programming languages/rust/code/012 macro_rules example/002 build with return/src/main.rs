
macro_rules! make_builder {
    ($name:ident, $builder:ident { $($field:ident : $ty:ty),* $(,)? }) => {
        #[derive(Debug)]
        struct $name {
            $($field: $ty,)*
        }

        #[derive(Default)]
        struct $builder {
            $($field: Option<$ty>),*
        }

        impl $builder {
            $(
                fn $field(mut self, value: $ty) -> Self {
                    self.$field = Some(value);
                    self
                }
            )*

            fn build(self) -> Result<$name, String> {
                let mut missing: Vec<&'static str> = Vec::new();
                $(
                    if self.$field.is_none() {
                        missing.push(stringify!($field));
                    }
                )*
                if !missing.is_empty() {
                    return Err(format!("not set field: {}", missing.join(", ")));
                }
                Ok($name {
                    $($field: self.$field.unwrap()),*
                })
            }
        }

        impl $name {
            fn builder() -> $builder {
                $builder::default()
            }
        }
    };
}

make_builder!(Person, PersonBuilder {
    name: String,
    age: u32,
    email: String,
});

fn main() {
    let p = Person::builder()
        .name("Pablo".to_string())
        .age(30)
        .email("example@example.com".to_string())
        .build();
    println!("p: {:?}", p);

    let broken = Person::builder()
        .name("Ghost".to_string())
        .build();
    println!("broken: {:?}", broken);
}