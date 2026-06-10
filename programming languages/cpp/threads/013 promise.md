---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

### std::promise

Пара к `std::future` — позволяет **вручную установить значение или исключение** из одного потока, которое получит другой через `future`.

`async` создаёт `promise` автоматически. `promise` нужен когда нужен явный контроль — например результат формируется не в одной функции.

### Связка promise → future

```cpp
std::promise<int> p;
std::future<int> f = p.get_future(); // получить future из promise

// в другом потоке
p.set_value(42);      // установить результат
// или
p.set_exception(std::make_exception_ptr(std::runtime_error("fail")));
```

### promise vs async

| |`async`|`promise`|
|---|---|---|
|Создание future|автоматически|вручную через `get_future()`|
|Установка значения|возврат из функции|`set_value()` явно|
|Гибкость|низкая|высокая|
|Использование|простые случаи|сложные pipeline, callback-и|

**Правило:** `async` когда результат — возврат функции. `promise` когда результат формируется в произвольный момент или передаётся через callback.

```cpp
#include <iostream>
#include <thread>
#include <future>

void compute(std::promise<int> _promise, int _value) {
    _promise.set_value(_value * _value);
}

void risky(std::promise<int> _promise) {
    try {
        throw std::runtime_error("sth went wrong");
    } catch(const std::exception& e) {
        _promise.set_exception(std::current_exception());
    }
}

void only_signal(std::promise<void> _promise) {
    _promise.set_value();
}

int main() {
    std::promise<int> p0;
    auto&& f0 = p0.get_future();
    std::thread t0{compute, std::move(p0), 10};
    t0.join();
    std::cout << "t0: " << f0.get() << std::endl;

    std::promise<int> p1;
    auto&& f1 = p1.get_future();
    std::thread t1{risky, std::move(p1)};
    t1.join();

    try {
        f1.get();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::promise<void> p2;
    auto&& f2 = p2.get_future();
    std::thread t2{only_signal, std::move(p2)};
    t2.join();
    f2.wait();
    std::cout << "Done" << std::endl;

    return 0;
}
```

```
t0: 100
sth went wrong
Done
```

---
---

## 🔹 Неделя 4: Современные паттерны и инструменты

**Цель:** Использовать высокоуровневые абстракции.

### Темы:
- `std::packaged_task` — оборачивание функций

### Практика:
```cpp
// Выполнить 3 тяжёлых вычисления асинхронно
// Собрать результаты через std::future
```

📚 Документация: `std::async`, `std::future`, `std::promise`

---

## 🔹 Неделя 5: Паттерны и безопасность

**Цель:** Писать безопасный и эффективный многопоточный код.

### Темы:
- Deadlock: причины и как избежать (алгоритм "упорядочения мьютексов")
- `std::lock()` — безопасная блокировка нескольких мьютексов
- RAII и исключения в потоках
- Thread-safe классы
- `thread_local` переменные
- Профилирование и отладка многопоточных программ

### Практика:
```cpp
// Реализовать thread-safe кэш (ключ → значение)
// Поддержка read/write, использовать shared_mutex
```

---

## 🔹 Неделя 6: Продвинутые темы (по желанию)

**Цель:** Расширить знания до продвинутого уровня.

### Темы:
- Пул потоков (thread pool) — реализация
- `std::jthread` (C++20) — автоматический `join`
- `std::stop_token`, `std::stop_source` (C++20) — безопасное завершение
- Lock-free структуры данных (на базе `std::atomic`)
- Работа с GUI или сетевыми серверами в многопоточной среде

### Практика:
```cpp
// Написать простой пул потоков с очередью задач
```

---

## 🛠️ Инструменты и среды

| Инструмент | Для чего |
|----------|---------|
| **g++ / clang++ с `-pthread`** | Компиляция многопоточных программ |
| **Valgrind + Helgrind/DRD** | Поиск race conditions |
| **GCC/Clang с `-fsanitize=thread`** | ThreadSanitizer — лучший выбор |
| **IDE: CLion, VS Code, Visual Studio** | Отладка потоков |

---

## 📚 Рекомендуемые источники

### Книги:
- **"C++ Concurrency in Action"** — *Anthony Williams* (лучшая книга по теме)
- **"Effective Modern C++"** — *Scott Meyers* (разделы про concurrency)

### Онлайн:
- [https://en.cppreference.com](https://en.cppreference.com) — официальная документация
- [https://www.modernescpp.com](https://www.modernescpp.com) — отличные статьи по concurrency
- YouTube: поиск по "C++ threads tutorial"

---

## ✅ Советы по обучению

| Совет | Почему |
|------|--------|
| Пишите код каждый день | Многопоточность требует практики |
| Используйте ThreadSanitizer | Находит ошибки, которые вы не увидите сами |
| Начинайте с простого | Не бросайтесь сразу в lock-free программирование |
| Тестируйте на разных платформах | Поведение может отличаться (Linux vs Windows) |
| Избегайте глобальных переменных | Они усложняют тестирование |

---

## 🎯 Финальный проект (по окончании курса)

> **Создать HTTP-сервер (упрощённый), который:**
> - Обрабатывает запросы в отдельных потоках
> - Имеет thread-safe кэш
> - Использует пул потоков
> - Поддерживает асинхронные операции

---

Если хочешь, могу:
- Прислать пошаговые уроки с примерами
- Подготовить тесты по каждой теме
- Показать, как отлаживать deadlock
- Составить таблицу совместимости (C++11, 17, 20)

📌 Просто скажи: "Да, хочу подробный урок по [тема]"!

Удачи в изучении многопоточности! 💪