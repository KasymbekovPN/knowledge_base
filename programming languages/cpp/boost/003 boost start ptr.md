---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/boost/_|<=]]

# Boost.SmartPtr

Boost.SmartPtr — это набор умных указателей. Разберу каждый по отдельности.

CMake: `find_package(Boost REQUIRED)` + `target_link_libraries(app PRIVATE Boost::boost)` (header-only интерфейс).

## 1. `boost::shared_ptr<T>`

Указатель с подсчётом ссылок (разделяемое владение). Прообраз `std::shared_ptr`.

### Конструкторы

|Конструктор|Описание|
|---|---|
|`shared_ptr()`|Пустой (хранит `nullptr`, счётчик = 0)|
|`shared_ptr(Y* p)`|Берёт во владение сырой указатель|
|`shared_ptr(Y* p, Deleter d)`|С пользовательским удалителем|
|`shared_ptr(Y* p, Deleter d, Allocator a)`|С удалителем и аллокатором|
|`shared_ptr(const shared_ptr& r)`|Копия (увеличивает счётчик)|
|`shared_ptr(shared_ptr&& r)`|Перемещение|
|`shared_ptr(const weak_ptr<Y>& r)`|Из weak_ptr (бросает `bad_weak_ptr`, если объект уничтожен)|
|`shared_ptr(std::unique_ptr<Y,D>&& r)`|Перенос владения из unique_ptr|
|`shared_ptr(const shared_ptr<Y>& r, T* p)`|Aliasing-конструктор (делит счётчик r, но указывает на p)|

### Методы

|Метод|Описание|
|---|---|
|`T* get() const`|Сырой указатель (без передачи владения)|
|`T& operator*() const`|Разыменование|
|`T* operator->() const`|Доступ к членам|
|`long use_count() const`|Число shared_ptr, владеющих объектом|
|`bool unique() const`|`true`, если `use_count() == 1` (устарело)|
|`explicit operator bool() const`|`true`, если указатель не пустой|
|`void reset()`|Освободить владение (счётчик−1)|
|`void reset(Y* p)`|Заменить управляемый объект|
|`void reset(Y* p, Deleter d)`|Заменить с удалителем|
|`void swap(shared_ptr& other)`|Обмен|

### Создание

```cpp
boost::make_shared<T>(args...)        // рекомендуется: одна аллокация
boost::allocate_shared<T>(alloc, args...)
```

---

## 2. `boost::weak_ptr<T>`

Слабая (не владеющая) ссылка на объект, управляемый `shared_ptr`. Разрывает циклические ссылки.

### Методы

|Метод|Описание|
|---|---|
|`weak_ptr()`|Пустой|
|`weak_ptr(const shared_ptr<Y>&)`|Из shared_ptr|
|`shared_ptr<T> lock() const`|Получить shared_ptr (пустой, если объект уничтожен)|
|`long use_count() const`|Число владеющих shared_ptr|
|`bool expired() const`|`true`, если объект уже уничтожен|
|`void reset()`|Сбросить|
|`void swap(weak_ptr&)`|Обмен|

```cpp
boost::weak_ptr<Node> w = sp;
if (auto sp2 = w.lock()) { /* объект жив, sp2 безопасен */ }
```

---

## 3. `boost::scoped_ptr<T>`

Эксклюзивное владение в пределах области видимости. **Некопируемый, неперемещаемый.** Аналог упрощённого `unique_ptr`.

### Методы

|Метод|Описание|
|---|---|
|`scoped_ptr()` / `scoped_ptr(T* p)`|Пустой / берёт владение|
|`T& operator*()` / `T* operator->()`|Доступ к объекту|
|`T* get() const`|Сырой указатель|
|`explicit operator bool() const`|Не пустой?|
|`void reset(T* p = nullptr)`|Заменить/освободить объект|
|`void swap(scoped_ptr&)`|Обмен|

> Нет `release()` (в отличие от `unique_ptr`) — нельзя «отпустить» владение.

---

## 4. `boost::scoped_array<T>`

Как `scoped_ptr`, но для массивов (`new[]` / `delete[]`).

|Метод|Описание|
|---|---|
|`T& operator[](std::ptrdiff_t i)`|Доступ по индексу|
|`T* get() const`|Сырой указатель|
|`void reset(T* p = nullptr)`|Заменить|
|`void swap(...)`|Обмен|

---

## 5. `boost::shared_array<T>`

Разделяемое владение массивом с подсчётом ссылок.

|Метод|Описание|
|---|---|
|`T& operator[](std::ptrdiff_t i)`|Доступ по индексу|
|`T* get() const`|Сырой указатель|
|`long use_count() const`|Счётчик ссылок|
|`void reset(T* p = nullptr)`|Заменить|

---

## 6. `boost::intrusive_ptr<T>`

Указатель со счётчиком, **встроенным в сам объект** (счётчик хранится в T, а не рядом). Эффективнее по памяти, но требует поддержки от типа.

### Требования

Тип `T` должен предоставить две свободные функции:

```cpp
void intrusive_ptr_add_ref(T* p);  // увеличить счётчик
void intrusive_ptr_release(T* p);  // уменьшить, удалить при 0
```

### Методы

|Метод|Описание|
|---|---|
|`intrusive_ptr()`|Пустой|
|`intrusive_ptr(T* p, bool add_ref = true)`|Из сырого указателя|
|`T* get() const`|Сырой указатель|
|`T& operator*()` / `T* operator->()`|Доступ|
|`void reset()` / `void reset(T* p)`|Сбросить/заменить|
|`explicit operator bool() const`|Не пустой?|

> В стандарте аналога нет — это уникальная возможность Boost.

---

## 7. `boost::enable_shared_from_this<T>`

База, позволяющая объекту безопасно получить `shared_ptr` на самого себя.

|Метод|Описание|
|---|---|
|`shared_ptr<T> shared_from_this()`|shared_ptr на этот объект|
|`weak_ptr<T> weak_from_this()`|weak_ptr на этот объект|

```cpp
struct Widget : boost::enable_shared_from_this<Widget> {
    boost::shared_ptr<Widget> self() { return shared_from_this(); }
};
auto w = boost::make_shared<Widget>(); // ОБЯЗАТЕЛЬНО через shared_ptr
auto s = w->self();
```

---

## 8. `boost::local_shared_ptr<T>`

Однопоточный `shared_ptr` без атомарного счётчика — быстрее, когда многопоточность не нужна. Создаётся через `boost::make_local_shared<T>(...)`. (В стандарте аналога нет.)

---

## Связанные сущности

| Имя                                      | Описание                                                        |
| ---------------------------------------- | --------------------------------------------------------------- |
| `boost::bad_weak_ptr`                    | Исключение при конструировании shared_ptr из истёкшего weak_ptr |
| `boost::make_shared<T>(...)`             | Создание shared_ptr одной аллокацией                            |
| `boost::allocate_shared<T>(...)`         | То же с пользовательским аллокатором                            |
| `boost::make_local_shared<T>(...)`       | Создание local_shared_ptr                                       |
| `boost::static_pointer_cast<T>(sp)`      | static_cast для shared_ptr                                      |
| `boost::dynamic_pointer_cast<T>(sp)`     | dynamic_cast для shared_ptr                                     |
| `boost::const_pointer_cast<T>(sp)`       | const_cast для shared_ptr                                       |
| `boost::reinterpret_pointer_cast<T>(sp)` | reinterpret_cast для shared_ptr                                 |
| `boost::get_deleter<D>(sp)`              | Доступ к удалителю shared_ptr                                   |

## Сводка: какой указатель выбрать

|Сценарий|Указатель|
|---|---|
|Разделяемое владение|`shared_ptr`|
|Не владеющая ссылка / разрыв циклов|`weak_ptr`|
|Эксклюзивное владение в scope|`scoped_ptr`|
|Эксклюзивный массив|`scoped_array`|
|Разделяемый массив|`shared_array`|
|Счётчик внутри объекта|`intrusive_ptr`|
|`shared_ptr` на себя изнутри класса|`enable_shared_from_this`|
|Разделяемое владение в один поток|`local_shared_ptr`|

## Отличия от стандартных умных указателей

- `boost::shared_ptr` ↔ `std::shared_ptr`; `boost::weak_ptr` ↔ `std::weak_ptr` — практически идентичны по API.
- `boost::scoped_ptr` ≈ упрощённый `std::unique_ptr` (но **без** `release()` и без перемещения). В новом коде обычно берут `std::unique_ptr`.
- `boost::scoped_array` / `shared_array` ↔ `std::unique_ptr<T[]>` / `std::shared_ptr<T[]>` (последнее с C++17).
- `boost::intrusive_ptr` и `boost::local_shared_ptr` — **аналогов в стандарте нет**, это весомые причины использовать Boost.
- `enable_shared_from_this` есть в обоих; в boost добавлен `weak_from_this()` (в стандарт пришёл в C++17).
- Касты: `boost::static_pointer_cast` и др. ↔ `std::static_pointer_cast` и др.

В современном коде для базовых сценариев предпочитают `std::`-указатели; Boost.SmartPtr остаётся ценным ради `intrusive_ptr`, `local_shared_ptr` и при поддержке старых кодовых баз.


### include/test_smart_ptr.h
```cpp
#pragma once  
  
#include <boost/smart_ptr.hpp>  
  
namespace test_smart_ptr {  
    struct Node {  
        int value;  
        boost::shared_ptr<Node> next;  
        boost::weak_ptr<Node> prev;  
        Node(int _value): value{_value} {}  
    };    void test();  
}
```

### src/test_smart_ptr.cpp
```cpp
#include "test_smart_ptr.h"  
  
#include <iostream>  
#include <format>  
  
namespace test_smart_ptr {  
  
void test() {  
    auto&& a = boost::make_shared<Node>(42);  
    auto&& b = boost::make_shared<Node>(12);  
  
    a->next = b;  
    b->prev = a;  
  
    std::cout << std::format("a.use_count: {}\n", a.use_count());  
    std::cout << std::format("b.use_count: {}\n", b.use_count());  
  
    if (auto p = b->prev.lock()) {  
        std::cout << std::format("prev value: {}\n", p->value);  
    }  
    boost::shared_ptr<void> v = a;  
    auto&& back = boost::static_pointer_cast<Node>(v);  
}  
  
}
```

```
a.use_count: 1
b.use_count: 2
prev value: 42
```
