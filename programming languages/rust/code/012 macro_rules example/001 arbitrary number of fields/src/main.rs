
macro_rules! make_builder_v2 {
    ($name:ident, $builder:ident { $($field:ident : $ty:ty), * $(,)? }) => {
        struct $name {
            $($field: $ty),*
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

            fn build(self) -> $name {
                $name {
                    $($field: self.$field.expect(concat!(stringify!($field), " not set"))),*
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

make_builder_v2!(Person, PersonBuilder {
    name: String,
    age: u32,
    email: String,
});

fn main() {
    let p = Person::builder()
        .name("Pablo".to_string())
        .age(30)
        .email("pablo@example.com".to_string())
        .build();
    println!("{} ({}) <{}>", p.name, p.age, p.email);
}