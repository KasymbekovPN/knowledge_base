---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

`thread_local` — спецификатор хранения, при котором **каждый поток имеет свою копию** переменной. Изменения в одном потоке не видны другим.

### Инициализация

Каждый поток инициализирует свою копию **при первом обращении**:

```cpp
thread_local std::vector<int> buffer; // каждый поток получит свой пустой вектор

thread_local int id = generate_id();  // generate_id() вызывается отдельно в каждом потоке
```

### Когда полезен

**Избежать мьютекса для потоко-специфичных данных:**

```cpp
// без thread_local — нужен мьютекс
std::mutex mtx;
std::string last_error;

// с thread_local — каждый поток хранит свою ошибку
thread_local std::string last_error;

void set_error(const std::string& msg) {
    last_error = msg; // нет гонки — у каждого своя копия
}
```

**Кэш внутри потока:**

```cpp
thread_local std::unordered_map<int, int> cache;

int compute(int x) {
    if (auto it = cache.find(x); it != cache.end())
        return it->second;
    return cache[x] = x * x;
}
```

### Время жизни

Копия переменной живёт **столько, сколько живёт поток**. При завершении потока — деструктор вызывается автоматически.

```cpp
struct Logger {
    Logger()  { std::cout << "Logger created\n"; }
    ~Logger() { std::cout << "Logger destroyed\n"; }
};

thread_local Logger logger; // создаётся и уничтожается с каждым потоком
```

### Ограничения

```cpp
thread_local int x = 0;

int* ptr = &x;  // указатель валиден только в том потоке, где взят
                // передача в другой поток — UB
```

```cpp
#include <iostream>
#include <thread>

class Logger {

public:
    Logger() {
        std::cout
            << "[" << std::this_thread::get_id() << "] Logger created\n";
    }

    ~Logger() {
        std::cout
            << "[" << std::this_thread::get_id() << "] Logger destroyed\n";
    }
};

thread_local size_t counter{};
thread_local Logger logger;

void worker () {
    ++counter;
    ++counter;
    std::cout
        << "[" << std::this_thread::get_id()
        << "] counter: " << counter << "\n";
}

int main() {
    std::thread t0{worker};
    std::thread t1{worker};

    t0.join();
    t1.join();

    return 0;
}
```

```
[4528] Logger created
[16112] Logger created
[532] Logger created
[16112] counter: 2
[532] counter: 2
[16112] Logger destroyed
[532] Logger destroyed
[4528] Logger destroyed
```
