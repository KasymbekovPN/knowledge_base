---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Шаг 1: минимальная версия — жёстко зашитые два поля

Начнём без обобщений вообще, чтобы увидеть голую механику pattern matching в `macro_rules!`, прежде чем усложнять синтаксис повторениями.

```rust
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
                $name {
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

make_builder_v1!(Person, PersonBuilder { name: String, age: u32 });

fn main() {
    let p = Person::builder()
        .name("Pablo".to_string())
        .age(30)
        .build();
    println!("{} is {}", p.name, p.age);
}
```

Вывод: `Pablo is 30`

**Что здесь происходит механически:**

- `$name:ident` — фрагмент-спецификатор, говорит компилятору "здесь ожидается идентификатор" (имя типа/переменной/функции — не любой текст, а именно синтаксически валидный идентификатор).
- `$f1:ident : $t1:ty` — пара "имя поля : тип поля"; `:ty` — отдельный спецификатор именно для типов, не для произвольных выражений.
- Тело макроса после `=>` — это буквально шаблон кода, куда компилятор подставит захваченные фрагменты вместо `$f1`, `$t1` и т.д. Всё, что не начинается с `$`, копируется в раскрытие дословно.
- `$builder` используется и как имя генерируемой структуры, и (через `$f1(mut self, ...)`) как имя метода — макрос спокойно генерирует **несколько разных элементов** (struct, impl, методы) из одного вызова, что для функции просто невозможно.

Уже на этом шаге видно первое ограничение: два поля зашиты в саму сигнатуру макроса (`$f1`, `$f2` — ровно два, не больше и не меньше). Для произвольного числа полей нужен другой инструмент — повторения.

## Шаг 2: обобщаем через `$(...)*` — произвольное число полей

```rust
macro_rules! make_builder_v2 {
    ($name:ident, $builder:ident { $($field:ident : $ty:ty),* $(,)? }) => {
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
```

Вывод: `Pablo (30) <pablo@example.com>`

**Разбор новых элементов:**

- `$($field:ident : $ty:ty),*` — "ноль или больше пар `field: type`, разделённых запятой". `*` — квантификатор "ноль или больше" (как в regex); есть ещё `+` ("один или больше") и `?` ("ноль или один").
- `$(,)?` в конце паттерна — отдельный трюк для поддержки висячей запятой после последнего поля (`email: String,` с запятой в конце вызова макроса). Без этого макрос отказался бы матчить вызов с трейлинг-запятой.
- **Важно:** повторение в паттерне (`$(...)*` слева от `=>`) и повторение в теле (`$(...)*` справа) должны совпадать по количеству итераций — макрос-система сама следит, чтобы `$field` и `$ty` разворачивались синхронно, парами, в каждой точке использования `$(...)*` .
- `stringify!($field)` — встроенный макрос, превращающий идентификатор обратно в строку на этапе компиляции (`stringify!(name)` → `"name"`), без рантайм-затрат. `concat!` склеивает строковые литералы тоже на этапе компиляции. Оба — примеры простейших встроенных `macro_rules!`-подобных механизмов, которые исторически появились раньше пользовательских макросов.

Это уже рабочий обобщённый билдер, но `expect("field not set")` при незаполненном поле — не очень информативно и падает с паникой на первом же незаполненном поле, даже если их несколько.

## Шаг 3: `build()` возвращает `Result` со списком всех незаполненных полей сразу

```rust
macro_rules! make_builder {
    ($name:ident, $builder:ident { $($field:ident : $ty:ty),* $(,)? }) => {
        #[derive(Debug)]
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

            fn build(self) -> Result<$name, String> {
                let mut missing: Vec<&'static str> = Vec::new();
                $(
                    if self.$field.is_none() {
                        missing.push(stringify!($field));
                    }
                )*
                if !missing.is_empty() {
                    return Err(format!("не заполнены поля: {}", missing.join(", ")));
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
        .email("pablo@example.com".to_string())
        .build();
    println!("{:?}", p);

    let broken = Person::builder()
        .name("Ghost".to_string())
        .build();
    println!("{:?}", broken);
}
```

Вывод:

```
Ok(Person { name: "Pablo", age: 30, email: "pablo@example.com" })
Err("не заполнены поля: age, email")
```

Обрати внимание — `missing` собирает **все** незаполненные поля за один проход по всем `$field` через повторение `$(...)*` в теле метода (это уже не разворачивание в код структуры, а разворачивание в последовательность `if`-проверок одна за другой) — макрос одинаково легко генерирует и данные, и императивную логику из одного и того же списка полей.

## Гигиена макроса — покажем на практике, не только в теории

Раньше обсуждали, что переменные внутри макроса не конфликтуют с одноимёнными снаружи. Проверим прямо на этом билдере:

```rust
fn main() {
    let value = "внешняя переменная, не related к макросу".to_string();
    let p = Person::builder()
        .name("Pablo".to_string())
        .age(30)
        .email("pablo@example.com".to_string())
        .build();
    println!("{value}"); // печатает исходную строку, макрос её не тронул
}
```

Внутри макроса параметр метода тоже называется `value` (`fn $field(mut self, value: $ty) -> Self`), но это **не та же переменная**, что снаружи — гигиена гарантирует изоляцию, даже несмотря на текстуальное совпадение имени.

## Сравнение с ручным Builder на C++ (из более раннего разговора)

Помнишь пример с `Builder` на `self`-методах? Вот в чём разница на практике: там для каждого нового поля пришлось бы вручную дописывать метод-сеттер и обновлять `build()`. Здесь же добавление нового поля в DSL:

```rust
make_builder!(Person, PersonBuilder {
    name: String,
    age: u32,
    email: String,
    phone: String, // просто добавили строку
});
```

автоматически порождает и `.phone(...)` метод, и проверку в `build()`, и поле в структуре — без единой правки самого макроса. Именно это и есть причина писать макрос вместо copy-paste: избавиться от повторяющегося, но не идентичного кода для каждого нового поля/структуры.

## Ограничение, с которым ты столкнёшься на практике

Заметь: пришлось передавать имя билдера (`PersonBuilder`) явно вторым аргументом. `macro_rules!` **не умеет** генерировать новый идентификатор из существующего (например, автоматически сделать `PersonBuilder` из `Person`, приклеив суффикс) — конкатенация идентификаторов на стабильном Rust в декларативных макросах невозможна. Это ровно то место, где начинается ниша **процедурных** макросов (`derive`-макросы через crate `syn`/`quote`, которые мы обсуждали): настоящий `#[derive(Builder)]` из крейта `derive_builder` в экосистеме сам генерирует имя `PersonBuilder` из `Person`, потому что процедурный макрос работает с произвольным анализом AST, а не только с pattern matching по токенам.
