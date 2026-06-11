---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

Класс потокобезопасен, если его можно использовать из нескольких потоков одновременно без гонок данных и неопределённого поведения.

### Уровни потокобезопасности

```cpp
// 1. Не потокобезопасен — любое разделение опасно
struct Counter {
    int value{};
    void increment() { ++value; } // гонка данных
};

// 2. Базовая безопасность — разные объекты можно использовать одновременно,
//    один объект — только из одного потока
std::vector<int> v; // разные векторы — ок, один — нет

// 3. Полная потокобезопасность — один объект из любого числа потоков
std::atomic<int> counter{0}; // всегда безопасно
```

### Чек-лист при проектировании

|Вопрос|Что проверить|
|---|---|
|Все поля защищены?|мьютекс на каждый изменяемый инвариант|
|Составные операции атомарны?|check-then-act в одном lock|
|Нет утечки ссылок?|методы возвращают копии, не ссылки|
|const-методы защищены?|мьютекс `mutable`|
|Один мьютекс на 2+ полях?|или `scoped_lock` при нескольких|


---



---

### Проектирование thread-safe класса

**Правило:** защищать нужно **инварианты**, а не просто отдельные операции.

```cpp
class ThreadSafeCounter {
public:
    void increment() {
        std::lock_guard lock(mtx);
        ++value;
    }

    void decrement() {
        std::lock_guard lock(mtx);
        --value;
    }

    int get() const {
        std::lock_guard lock(mtx);
        return value;
    }

private:
    mutable std::mutex mtx;
    int value{};
};
```

`mutable` — позволяет захватывать мьютекс в `const`-методах.

---

### Ловушка — составные операции

Защита каждой операции по отдельности **не делает** их комбинацию безопасной:

```cpp
// каждый метод защищён, но проверка + действие — гонка
if (!stack.empty()) {    // поток A: стек не пуст
    stack.pop();         // поток B: уже забрал элемент → UB
}
```

Решение — предоставлять составные операции как единую транзакцию:

```cpp
class ThreadSafeStack {
public:
    std::optional<int> pop() {
        std::lock_guard lock(mtx);
        if (data.empty()) return std::nullopt;
        int val = data.top();
        data.pop();
        return val;
    }

    void push(int val) {
        std::lock_guard lock(mtx);
        data.push(val);
    }

private:
    std::mutex mtx;
    std::stack<int> data;
};
```

---

### Избегать передачи внутренних данных наружу

```cpp
class Safe {
public:
    // ОПАСНО — возвращает ссылку на защищённые данные
    const std::string& get_name() const {
        std::lock_guard lock(mtx);
        return name; // мьютекс освобождён, но ссылка жива
    }

    // ПРАВИЛЬНО — копия
    std::string get_name() const {
        std::lock_guard lock(mtx);
        return name;
    }

private:
    mutable std::mutex mtx;
    std::string name;
};
```

---

### Читатели и писатели — shared_mutex

```cpp
class ThreadSafeConfig {
public:
    std::string get(const std::string& key) const {
        std::shared_lock lock(mtx);        // много читателей
        auto it = data.find(key);
        return it != data.end() ? it->second : "";
    }

    void set(const std::string& key, const std::string& val) {
        std::unique_lock lock(mtx);        // один писатель
        data[key] = val;
    }

private:
    mutable std::shared_mutex mtx;
    std::unordered_map<std::string, std::string> data;
};
```

---



---
---
---
- `thread_local` переменные
- Профилирование и отладка многопоточных программ

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