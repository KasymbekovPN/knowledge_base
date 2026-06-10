---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### std::async

Запускает функцию **асинхронно** и возвращает `std::future` — объект, через который можно получить результат когда он будет готов. Более высокоуровневая абстракция чем `std::thread`.

### Политики запуска

```cpp
// запустить в отдельном потоке немедленно
std::async(std::launch::async, func);

// отложить до вызова .get() или .wait() — в том же потоке
std::async(std::launch::deferred, func);

// решает реализация (по умолчанию)
std::async(func);
```

> **Важно:** без явной политики компилятор может выбрать `deferred` — реального параллелизма не будет. Лучше всегда указывать `std::launch::async`.

### Методы std::future

| Метод                     | Описание                                                                           |
| ------------------------- | ---------------------------------------------------------------------------------- |
| `f.get()`                 | Получить результат. Блокирует до готовности. Можно вызвать **только один раз**     |
| `f.wait()`                | Ждать готовности без получения результата                                          |
| `f.wait_for(duration)`    | Ждать не дольше указанного времени, вернуть статус                                 |
| `f.wait_until(timepoint)` | Ждать до указанного момента времени, вернуть статус                                |
| `f.valid()`               | Проверить, связан ли `future` с результатом (до `get()` — `true`, после — `false`) |
### Статусы wait_for / wait_until

```cpp
std::future_status::ready     // результат готов
std::future_status::timeout   // время вышло, результат не готов
std::future_status::deferred  // задача отложена (launch::deferred), ещё не запущена
```

### valid() — важный нюанс

```cpp
auto f = std::async(std::launch::async, func);

f.valid(); // true
f.get();
f.valid(); // false — повторный get() бросит std::future_error
```

### Обработка исключений

Исключение из асинхронной функции **не теряется** — пробрасывается при вызове `.get()`:

```cpp
auto f = std::async(std::launch::async, []() {
    throw std::runtime_error("oops");
});

try {
    f.get();
} catch (const std::exception& e) {
    std::cout << e.what() << "\n"; // "oops"
}
```

### async vs thread

|                  | `std::thread`            | `std::async`                  |
| ---------------- | ------------------------ | ----------------------------- |
| Возврат значения | нет                      | да — через `future`           |
| Исключения       | обрывают программу       | пробрасываются через `future` |
| Управление       | ручное (`join`/`detach`) | автоматическое                |
| Уровень          | низкий                   | высокий                       |

**Правило:** если нужен результат из потока — `async`. Если нужен тонкий контроль над потоком — `thread`.

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <numeric>

int sum(std::vector<int>::iterator _begin,
        std::vector<int>::iterator _end) {
    return std::accumulate(_begin, _end, 0);
}

int main() {
    std::vector<int> buffer(1000000, 1);
    auto&& mid = buffer.begin() + buffer.size() / 2;

    auto&& f0 = std::async(std::launch::async, sum, buffer.begin(), mid);
    auto&& f1 = std::async(std::launch::async, sum, mid, buffer.end());

    std::cout << f0.get() + f1.get() << std::endl;

    return 0;
}
```

```
1000000
```
