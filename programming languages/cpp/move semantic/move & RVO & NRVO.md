---
tags:
  - programming-language
  - cpp
  - move-semantic
---
[[programming languages/cpp/move semantic/_|<=]]

В C++ существует несколько механизмов, которые позволяют избежать лишних копирований объектов при возврате из функции.

| Термин | Полное название | Описание |
|-------|------------------|----------|
| **RVO** | Return Value Optimization | Устраняет временный объект при возврате из функции |
| **NRVO** | Named Return Value Optimization | То же, что и RVO, но для именованного объекта |
| **Move** | Move Semantics | "Перемещает" ресурсы вместо копирования |

**RVO (Return Value Optimization)** — это оптимизация компилятора, которая **удаляет лишнее копирование или перемещение** при возврате объекта из функции.

```cpp
MyClass create() {
    return MyClass(); // Temporary object → RVO
}
```

- Компилятор создаёт объект сразу на месте вызова.
- Не вызывается ни copy, ни move конструктор.

> ✅ Это самая эффективная оптимизация — **ничего не копируется и не перемещается**.

**NRVO (Named Return Value Optimization)** — это та же RVO, но применяется к **именованному локальному объекту**.

```cpp
MyClass create() {
    MyClass obj;
    return obj; // Named object → NRVO
}
```

- Объект `obj` создаётся непосредственно в памяти принимающего объекта.
- Также **не вызываются copy/move**.

> ⚠️ NRVO **не гарантируется стандартом**, хотя большинство современных компиляторов его поддерживают.

Если RVO/NRVO **не сработали**, то будет использован **move семантика**, если тип поддерживает __move__.

```cpp
MyClass create() {
    MyClass obj;
    return std::move(obj); // explicit move
}
```

- Вызывается **move конструктор**.
- Быстрее __copy__, потому что "перемещает" ресурсы, а не копирует их.
- Может быть **замедлен**, если вы явно используете `std::move`, так как это **мешает RVO**.

```cpp
#include <iostream>

using std::cout;
using std::endl;

class Demo {
public:
    Demo() {cout << "ctor" <<endl;}
    Demo(const Demo&) {cout << "copy ctor" << endl;}
    Demo(const Demo&&) noexcept {cout << "move ctor" << endl;}
};

Demo create_rvo() {
    return Demo();
}

Demo create_nvro() {
    Demo obj;
    return obj;
}

Demo create_move() {
    Demo obj;
    return std::move(obj);
}

int main() {
    cout << "Call create_rvo" << endl;
    create_rvo();

    cout << "Call create_nrvo" << endl;
    create_nvro();

    cout << "Call create_move" << endl;
    create_move();

    return 0;
}
```

```
Call create_rvo
ctor
Call create_nrvo
ctor
Call create_move
ctor
move ctor
```

| Особенность | Объяснение |
|------------|-------------|
| RVO/NRVO быстрее move | Потому что **ничего не копируется и не перемещается** |
| `std::move` мешает RVO | Если вы делаете `return std::move(obj);`, RVO отключается |
| NRVO не гарантировано | Может зависеть от компилятора и сложности функции |
| Move безопаснее copy | Если RVO/NRVO не применимы, move лучше copy |

##  Лучшая практика

| Ситуация | Что делать |
|----------|-------------|
| Возвращаешь временный объект | Просто `return T(...);` |
| Возвращаешь локальную переменную | `return obj;` — дай шанс RVO |
| Хочешь максимальную производительность | Не используй `std::move` при возврате |
| Нужна совместимость со старыми компиляторами | Реализуй move семантику на случай, если RVO не сработает |
| Пишешь библиотеку | Предполагай, что RVO может не сработать |

```cpp
#include <iostream>
#include <chrono>
#include <vector>

using namespace std::chrono;
using std::cout;
using std::endl;
using std::string;

std::vector<int> _get_temp_vector_nrvo(bool, const size_t);
std::vector<int> _get_temp_vector_move(bool, const size_t);
void _print_duration(steady_clock::time_point&,
					 steady_clock::time_point&,
					 string);

int main() {
    auto start_nrvo_1m = high_resolution_clock::now();
    volatile auto vec_nrvo_1m = _get_temp_vector_nrvo(true, 1'000'00);
    auto end_nrvo_1m = high_resolution_clock::now();

    auto start_move_1m = high_resolution_clock::now();
    volatile auto vec_move_1m = _get_temp_vector_move(true, 1'000'00);
    auto end_move_1m = high_resolution_clock::now();


    auto start_nrvo_10m = high_resolution_clock::now();
    volatile auto vec_nrvo_10m = _get_temp_vector_nrvo(true, 10'000'00);
    auto end_nrvo_10m = high_resolution_clock::now();

    auto start_move_10m = high_resolution_clock::now();
    volatile auto vec_move_10m = _get_temp_vector_move(true, 10'000'00);
    auto end_move_10m = high_resolution_clock::now();
  
    _print_duration(start_nrvo_1m, end_nrvo_1m, "NRVO 1M");
    _print_duration(start_move_1m, end_move_1m, "MOVE 1M");
    _print_duration(start_nrvo_10m, end_nrvo_10m, "NRVO 10M");
    _print_duration(start_move_10m, end_move_10m, "MOVE 10M");

    return 0;
}

std::vector<int> _get_temp_vector_nrvo(bool first, const size_t size) {
    std::vector<int> first_vector(size);
    std::vector<int> second_vector(size);

    // NRVO
    if (first) {
        return first_vector;
    } else {
        return second_vector;
    }
}

std::vector<int> _get_temp_vector_move(bool first, const size_t size) {
    std::vector<int> first_vector(size);
    std::vector<int> second_vector(size);
  
    if (first) {
        return std::move(first_vector);
    } else {
        return std::move(second_vector);
    }
}

void _print_duration(steady_clock::time_point &start,
                     steady_clock::time_point &end,
                     string label) {
    auto delta = end - start;
    std::cout
        << "[" << label <<"] Time: "
        << duration_cast<std::chrono::nanoseconds>(delta).count()
        << " ns" << std::endl;
}
```

```
[NRVO 1M] Time: 195800 ns
[MOVE 1M] Time: 106800 ns
[NRVO 10M] Time: 1023300 ns
[MOVE 10M] Time: 1106600 ns
```
