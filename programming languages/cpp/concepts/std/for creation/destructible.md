---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::destructible<T>`** из заголовка `<concepts>` проверяет, что тип `T` можно **безопасно уничтожить**, то есть выражение `t.~T()` корректно.
## 📘 Что означает `std::destructible<T>`?

Тип `T` удовлетворяет концепту, если:
- У него есть **доступный деструктор** (не `= delete`, не `private`),
- И его можно вызвать.

✅ Это базовое требование для любого типа, который:
- Можно хранить в переменной,
- Передавать в функцию,
- Хранить в контейнерах (`vector`, `unique_ptr` и т.д.).

### Важные моменты

| Особенность                  | Пояснение                                                |
| ---------------------------- | -------------------------------------------------------- |
| `void`                       | ❌ Не `destructible` — невозможно создать объект `void`   |
| `const T`, `volatile T`      | ✅ Если `T` destructible, то и квалифицированный тип тоже |
| Массивы                      | ✅ `int[5]` — destructible                                |
| Функции                      | ❌ Нельзя создать объект функции → не destructible        |
| `T[]` (незавершённый массив) | ❌ Не destructible — не может быть объектом               |

---

### Лучшая практика

| Совет                                                              | Почему                                |
| ------------------------------------------------------------------ | ------------------------------------- |
| Используйте `std::destructible` как базовое ограничение в шаблонах | Безопасность                          |
| Не пытайтесь создавать объекты с `private`/`delete` деструктором   | Будет ошибка                          |
| В библиотечном коде добавляйте `static_assert(destructible<T>)`    | Защита от ошибок                      |
| Предпочитайте RAII-обёртки (`unique_ptr`, `shared_ptr`)            | Они гарантируют корректное разрушение |


```cpp
#include <iostream>
#include <concepts>
#include <memory>
#include <vector>
#include <string>

class PrivDestructor {
private:
    ~PrivDestructor() = default;
public:
    static PrivDestructor create() { return PrivDestructor(); }
};


class DeletedDestructor {
public:
    ~DeletedDestructor() = delete;
};

template <typename T>
class SafeContainer {
private:
    T* data_;
    size_t size_;

public:
    static SafeContainer<T> create(size_t n_) {
        return SafeContainer<T>(n_);
    }

    explicit SafeContainer(size_t _n):
        data_{new T[_n]},
        size_{_n} {}

    ~SafeContainer() {
        std::cout << "DCTOR" << std::endl;
        delete[] data_;
    }
};

template<std::destructible T>
void test(T);

int main() {
    test(42);
    test(std::string("hello"));
    test(std::vector<int>{1, 2, 3});
    test(SafeContainer<int>::create(5));
    // test(PrivDestructor::create()); // Error

    return 0;
}

template<std::destructible T>
void test(T value) {
    std::cout << typeid(value).name() << std::endl;
}
```

```
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
class std::vector<int,class std::allocator<int> >
class SafeContainer<int>
DCTOR
```
