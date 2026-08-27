---
tags:
  - programming-language
  - rust
---
[[programming languages/rust/_|<=]]

## Точка входа — `fn main()`

Самый простой случай, прямой аналог `int main()` в C++:

```rust
fn main() {
    println!("Hello, world!");
}
```

Отличия от C++, которые сразу бросаются в глаза:

**1. `main` может возвращать `Result`.** Это удобно, когда точка входа сама может завершиться ошибкой (`?` работает прямо в `main`):

```rust
use std::fs::File;

fn main() -> Result<(), std::io::Error> {
    let _f = File::open("config.toml")?; // если ошибка — main вернёт Err, процесс завершится с ненулевым кодом
    Ok(())
}
```

Это заменяет паттерн C++, где обычно всё оборачивают в try/catch внутри `main` вручную:

```cpp
int main() {
    try {
        // ...
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }
}
```

**2. Явный код возврата** — через `std::process::exit(code)`, если нужен произвольный код, а не просто 0/1 от `Result`:

```rust
std::process::exit(2);
```

**3. Аргументы командной строки** — не параметры `main`, а отдельная функция:

```rust
fn main() {
    let args: Vec<String> = std::env::args().collect();
    println!("{:?}", args);
}
```

**4. Нет глобальной статической инициализации в духе C++** (static objects с конструкторами, выполняющимися до `main`, с неопределённым порядком между translation units — классическая головная боль). В Rust `static`/`const` инициализируются на этапе компиляции, без рантайм-конструкторов, так что static initialization order fiasco просто не существует как проблема.
