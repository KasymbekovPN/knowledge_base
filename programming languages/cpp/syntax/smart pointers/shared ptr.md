---
tags:
  - programming-language
  - cpp
  - syntax
  - smart-pointer
---
[[__cpp syntax smart ptr__|<==]]

Тип `shared_ptr<T>` применяется для создания указателей на объекты, на которые может указывать несколько указателей. `shared_ptr<T>` позволяет создавать множество объектов `shared_ptr<T>`, которые содержат один и тот же адрес.

Для данных указателей применяется механизм подсчета ссылок (_reference counting_). Каждый раз, когда создается объект `shared_ptr<T>`, увеличивается счетчик объектов `shared_ptr<T>`, которые содержат определенный адрес. Когда объект `shared_ptr<T>` удаляется или ему присваивается другой адрес, счетчик ссылок уменьшается на единицу. Когда больше нет объектов `shared_ptr<T>`, которые ссылаются на определенный адрес, счетчик ссылок сбрасывается в ноль.

```cpp
#include <iostream>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    shared_ptr<int> null_ptr;
    cout << "null ptr <= " << null_ptr << endl << endl;

    shared_ptr<int> ptr0 {make_shared<int>(42)};
    shared_ptr<int> ptr1 {ptr0};
    cout << ptr0 << " <> " << *ptr0 << endl;
    cout << ptr1 << " <> " << *ptr1 << endl << endl;

    *ptr0 = 123;
    cout << ptr0 << " <> " << *ptr0 << endl;
    cout << ptr1 << " <> " << *ptr1 << endl << endl;

    const size_t SIZE{5};
    auto array {make_shared<int[]>(SIZE)};
    array[2] = 42;

    cout << "array <= ";
    for (size_t i{}; i < SIZE; i++){
        cout << array[i] << " ";
    }
    cout << endl;

    return 0;
}
```

```
null ptr <= 0000000000000000

000001FB69749E70 <> 42
000001FB69749E70 <> 42

000001FB69749E70 <> 123
000001FB69749E70 <> 123

array <= 0 0 42 0 0
```

При определении указателя без явной инициализации по умолчанию он инициализируется значением `nullptr`.

Для инициализации конкретным значением можно применять функцию `std::make_shared<T>`.

Далее через указатель `shared_ptr` можно также, как и через обычный указатель, получать и изменять динамический объект.

Начиная со стандарта __C++20__ указатель `shared_ptr` может указывать на массив.

---
[shared_ptr](## shared_ptr)