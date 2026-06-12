---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

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

**Правило:** защищать нужно **инварианты**, а не просто отдельные операции.

`mutable` — позволяет захватывать мьютекс в `const`-методах.

```cpp
#include <iostream>
#include <mutex>

class Value {

public:
    int inc() {
        std::lock_guard lock{mtx};
        int old_value = value;
        ++value;
        ++counter;

        return old_value;
    }

    int dec() {
        std::lock_guard lock{mtx};
        int old_value = value;
        --value;
        ++counter;

        return old_value;
    }

    int get() const {
        std::lock_guard lock{mtx};
        return value;
    }

    size_t getCounter() const {
        std::lock_guard lock{mtx};
        return counter;
    }

private:
    mutable std::mutex mtx;
    int value{};
    size_t counter{};
};

std::ostream& operator<<(std::ostream& _os, const Value& _value) {
    return _os
        << "{value: " << _value.get()
        << ", counter: " << _value.getCounter() << "}";
}

int main() {
    Value value;
    std::thread{[&]() {value.inc();}}.join();
    std::thread{[&]() {value.dec();}}.join();
    std::thread{[&]() {value.inc();}}.join();

    std::cout << "value: " << value << std::endl;

    return 0;
}
```

```
value: {value: 1, counter: 3}
```

### Ловушка — составные операции

Защита каждой операции по отдельности **не делает** их комбинацию безопасной:

```cpp
if (!stack.empty()) {
    stack.pop();
}
```

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <stack>
#include <optional>
#include <condition_variable>

class Stack {

public:
    void push(int _value) {
        {
            std::lock_guard lock{mtx};
            data.push(_value);
        }
        cv.notify_one();
    }

    int pop() {
        std::unique_lock lock{mtx};
        cv.wait(lock, [this]() {
            return !data.empty();
        });

        int result = data.top();
        data.pop();

        return result;
    }

private:
    std::mutex mtx;
    std::stack<int> data;
    std::condition_variable cv;
};

int main() {
    Stack st;

    std::thread t0{[&]() {
        std::cout << "T0: " << st.pop() << "\n";
    }};

    std::thread t1{[&]() {
        st.push(42);
    }};

    t0.join();
    t1.join();

    return 0;
}
```

```
T0: 42
```

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

### Читатели (shared_lock) и писатели (unique_lock)

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
