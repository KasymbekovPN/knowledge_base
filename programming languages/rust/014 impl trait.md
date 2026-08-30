---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Производные трейты (`#[derive(...)]`)

Уже видели `#[derive(Debug, Clone, Default)]` мельком. Разберём, что конкретно генерируется, и на какие грабли натыкаются.

```rust
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
struct Version {
    major: u32,
    minor: u32,
    patch: u32,
}

fn main() {
    let v1 = Version { major: 1, minor: 2, patch: 3 };
    let v2 = v1.clone();
    println!("{:?}", v1);                                       // Debug
    println!("{}", v1 == v2);                                    // PartialEq
    println!("{}", v1 < Version { major: 1, minor: 3, patch: 0 }); // PartialOrd

    use std::collections::HashSet;
    let mut set = HashSet::new();
    set.insert(v1.clone());
    set.insert(v2);
    println!("{}", set.len()); // Hash + Eq нужны для HashSet

    let mut versions = vec![
        Version { major: 2, minor: 0, patch: 0 },
        Version { major: 1, minor: 0, patch: 0 },
    ];
    versions.sort(); // Ord нужен для sort()
    println!("{:?}", versions);
    println!("{:?}", Version::default());
}
```

Вывод:

```
Version { major: 1, minor: 2, patch: 3 }
true
true
1
[Version { major: 1, minor: 0, patch: 0 }, Version { major: 2, minor: 0, patch: 0 }]
Version { major: 0, minor: 0, patch: 0 }
```

Каждый `derive` — это конкретный процедурный derive-макрос (мы разбирали механику в теме про макросы), который генерирует поверхностную (structural) реализацию: `PartialEq` сравнивает поля по порядку, `Ord` сортирует по порядку полей (`major`, потом `minor`, потом `patch` — лексикографически, как `std::tuple`'s `operator<` в C++), `Hash` хеширует поля по очереди, `Debug` печатает имя struct и все поля.

**Аналог в C++** — это не одна фича, а разрозненный набор: генерируемые компилятором конструктор копирования / деструктор (частично покрывает `Clone`/`Drop`), ручной `operator==`/`operator<=>` (C++20 умеет генерировать `<=>` через `= default`, что ближе всего к `derive(PartialOrd)`), отдельная специализация `std::hash<T>` для использования в `unordered_map`, и ручной `operator<<` для вывода. В Rust всё это — единая, единообразная механика через один и тот же атрибут.

### Грабли №1: derive требует, чтобы КАЖДОЕ поле тоже реализовывало трейт

```rust
#[derive(Debug)]
struct NotComparable;

#[derive(Debug, PartialEq)]
struct Wrapper {
    value: NotComparable, // NotComparable не реализует PartialEq
}
```

Ошибка:

```
error[E0369]: binary operation `==` cannot be applied to type `NotComparable`
note: an implementation of `PartialEq` might be missing for `NotComparable`
help: consider annotating `NotComparable` with `#[derive(PartialEq)]`
```

Механически это ожидаемо: `derive(PartialEq)` генерирует что-то вроде `self.value == other.value`, а для этого сравнения нужен `PartialEq` у самого `NotComparable`. Fix — добавить `#[derive(PartialEq)]` и на `NotComparable` тоже (компилятор сам это подсказывает в `help`).

### Грабли №2: derive на generic-структуре добавляет bound автоматически, даже когда он реально не нужен

Это более тонкая и известная проблема (её иногда называют "derive macro over-constraining"):

```rust
use std::marker::PhantomData;

#[derive(Clone)]
struct Wrapper<T> {
    _marker: PhantomData<T>, // T реально нигде не хранится и не клонируется
    value: i32,
}

struct NotClone; // намеренно без Clone

fn main() {
    let w: Wrapper<NotClone> = Wrapper { _marker: PhantomData, value: 5 };
    let w2 = w.clone(); // ОШИБКА
}
```

Ошибка:

```
error[E0599]: the method `clone` exists for struct `Wrapper<NotClone>`,
but its trait bounds were not satisfied
doesn't satisfy `NotClone: Clone`
```

Хотя `NotClone` физически не клонируется (он спрятан в `PhantomData`, который сам по себе тривиально `Clone` независимо от `T`), макрос `derive(Clone)` наивно генерирует `impl<T: Clone> Clone for Wrapper<T>`, требуя `T: Clone` **всегда**, а не только когда это реально необходимо. Это известное ограничение derive-макросов из стандартной библиотеки — обходится либо ручной реализацией `Clone` без лишнего bound'а, либо крейтом типа `derivative`/`derive-where`, которые умеют генерировать более точные bounds. Стоит просто знать, что "derive не думает", а действует по шаблону — если натыкаешься на такую ошибку с generic-структурой, это первое, что стоит проверить.

## `impl Trait` в позиции аргумента (APIT — argument-position impl Trait)

```rust
use std::fmt::Display;

fn print_it(item: impl Display) {
    println!("{item}");
}
```

Это **чистый синтаксический сахар** для:

```rust
fn print_it_generic<T: Display>(item: T) {
    println!("{item}");
}
```

Компилятор внутри превращает первое ровно во второе — никакой разницы в генерируемом коде, мономорфизация работает одинаково. Разница чисто синтаксическая: `impl Trait` короче для простых случаев, но **не даёт** вызывающему коду возможность указать тип явно через turbofish:

```rust
print_it_generic::<i32>(42); // можно явно указать T
print_it(42);                 // impl Trait — так нельзя, тип выводится только из аргумента
```

Так что `impl Trait` в аргументе — это "сахар для generic, когда явное указание типа снаружи не нужно". Для сложных сигнатур с несколькими одинаковыми ограничениями явный `<T: Trait>` часто читается яснее (плюс позволяет связать несколько параметров одним и тем же типом `T`, чего `impl Trait` сделать не может — два `impl Display` в параметрах это два **независимых** скрытых типа, даже если по факту передашь одинаковые).

## `impl Trait` в позиции возврата (RPIT — return-position impl Trait)

Это уже частично разбирали в теме про замыкания — сейчас закроем полностью, с итераторами как более общим примером.

```rust
fn make_adder(x: i32) -> impl Fn(i32) -> i32 {
    move |y| x + y
}

fn evens_up_to(n: i32) -> impl Iterator<Item = i32> {
    (0..n).filter(|x| x % 2 == 0)
}

fn main() {
    let add5 = make_adder(5);
    println!("{}", add5(10)); // 15

    let v: Vec<i32> = evens_up_to(10).collect();
    println!("{:?}", v); // [0, 2, 4, 6, 8]
}
```

Ключевая идея: `impl Iterator<Item = i32>` в возврате означает "какой-то конкретный тип, реализующий `Iterator<Item = i32>`, но я не хочу писать его настоящее имя". А настоящее имя здесь монструозное — что-то вроде `Filter<Range<i32>, {closure@src.rs:8:20}>` — тип, зависящий от анонимного типа замыкания внутри `filter`, который **невозможно** написать руками в сигнатуре функции. `impl Trait` в возврате решает именно эту проблему: тип есть, он конкретный и известен компилятору (это статическая диспетчеризация — ноль overhead, инлайнится), просто скрыт от вызывающего кода (opaque type).

Аналог в C++ — auto-возврат (C++14) в связке с ranges/views:

```cpp
auto evens_up_to(int n) {
    return std::views::iota(0, n) | std::views::filter([](int x) { return x % 2 == 0; });
}
```

Оба решают одну и ту же проблему — невозможность (или практическую неудобность) выписать тип композиции ленивых адаптеров руками.

## Ограничение RPIT: только один конкретный тип на все пути возврата

Вот где это ловит новичков:

```rust
fn make_iter(flag: bool) -> impl Iterator<Item = i32> {
    if flag {
        0..5                          // Range<i32>
    } else {
        (10..20).step_by(2)           // StepBy<Range<i32>> — ДРУГОЙ конкретный тип!
    }
}
```

Ошибка:

```
error[E0308]: `if` and `else` have incompatible types
```

`impl Trait` в возврате — это **один** скрытый, но конкретный тип, зафиксированный на этапе компиляции. `Range<i32>` и `StepBy<Range<i32>>` — разные типы, даже если оба реализуют `Iterator<Item = i32>` — точно так же, как разные замыкания в примере с `make_op` из темы про `Fn`. Fix — тот же самый принцип, что и там: `Box<dyn Trait>`, когда конкретный тип должен различаться в зависимости от ветки:

```rust
fn make_iter(flag: bool) -> Box<dyn Iterator<Item = i32>> {
    if flag {
        Box::new(0..5)
    } else {
        Box::new((10..20).step_by(2))
    }
}
```

Вывод (обе ветки):

```
0
1
2
3
4
10
12
14
16
18
```

Здесь уже динамическая диспетчеризация — небольшой overhead на vtable и heap-аллокацию под `Box`, но зато один и тот же возвращаемый тип покрывает произвольное число разных конкретных реализаций `Iterator`.

## `impl Trait` нельзя использовать как тип поля структуры

```rust
struct Holder {
    item: impl Display, // ОШИБКА: impl Trait не разрешён здесь
}
```

Это прямое следствие того, что `impl Trait` — не настоящий именованный тип, а способ сказать компилятору "выведи и подставь сюда конкретный тип сам, в контексте одной конкретной функции". Структуры требуют явного, именуемого типа для каждого поля — либо конкретный тип, либо generic-параметр (`struct Holder<T: Display> { item: T }`), либо `Box<dyn Display>`, если нужна гетерогенность.

## Сводная таблица: где что применимо

|Позиция|Конструкция|Диспетчеризация|Overhead|Множественные конкретные типы|
|---|---|---|---|---|
|Аргумент|`impl Trait`|статическая|нет|сахар для `<T: Trait>`, полностью эквивалентно|
|Аргумент|`&dyn Trait`|динамическая|vtable|да, естественно|
|Возврат|`impl Trait`|статическая|нет|нет — один конкретный тип на всю функцию|
|Возврат|`Box<dyn Trait>`|динамическая|vtable + heap-аллокация|да|
|Поле структуры|`impl Trait`|—|—|недопустимо синтаксически|
|Поле структуры|generic `<T: Trait>`|статическая (per instantiation)|нет|нет, T фиксирован для конкретного instantiation|
|Поле структуры|`Box<dyn Trait>`|динамическая|vtable + heap|да|

Резюмируя оба вопроса одной мыслью: и `derive`, и `impl Trait` — это в первую очередь **экономия печати**, которая по-разному ломается на границах — `derive` ломается, когда поле/generic-параметр не удовлетворяет требуемому трейту, `impl Trait` ломается, когда одна сигнатура должна покрывать несколько разных конкретных типов. В обоих случаях выход — либо ослабить требование (не деривить, писать вручную; не `impl Trait`, а `dyn Trait`), либо удовлетворить его honestly (добавить недостающий derive на вложенный тип; свести все ветки к одному конкретному типу).
