---
tags:
  - programming-language
  - cpp
  - move-semantic
---
[[programming languages/cpp/move semantic/_|<=]]

Возвращение **rvalue из функции** — это важный аспект **move семантики** в C++. Он позволяет эффективно передавать временные объекты, не вызывая лишних копирований.

```cpp
CustomClass create() {
	CustomClass obj;
	return obj; // return local object until C++11
}
```

В __C++11__ и новее:
- Компилятор **может автоматически применить move**, если тип поддерживает move-семантику.
- Часто срабатывает оптимизация **RVO (Return Value Optimization)** или **NRVO (Named Return Value Optimization)**, и объект создаётся сразу на месте вызова — без копирования и _move_.

> ✅ То есть: даже если вы просто возвращаете `obj`, компилятор может его **не копировать и не перемещать**.

Если вы хотите явно указать, что вы хотите "переместить" объект (например, для ясности или если RVO отключено), можно использовать `std::move`:

```cpp
CustomClass create() {
	CustomClass obj;
	return std::move(obj); // return rvalue
}
```

Однако:
> ❗ Это **не ускорит код**, а может даже **запретить RVO**, так как теперь возвращается rvalue (`std::move(obj)`), а не именованный объект.

Поэтому:
> 🔁 **Рекомендуется возвращать объект напрямую**, без `std::move`.

### Когда применяется move при возврате?

| Ситуация                       | Применяется ли move                                        |
| ------------------------------ | ---------------------------------------------------------- |
| Возврат временного объекта     | ✅ Да                                                       |
| Возврат локальной переменной   | ✅ Может быть применён move, но чаще сработает RVO          |
| Возврат через `std::move(obj)` | ✅ Move применяется, но **может мешать RVO**                |
| Возврат ссылки (`T&&`)         | ❌ Не рекомендуется — возвращает ссылку на локальный объект |
```cpp
#include <iostream>
#include <utility>

using std::cout;
using std::endl;

class CustomContainer {

private:
    size_t size;
    int* data;

public:
    CustomContainer(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "CC ctor" << endl;
    }

    CustomContainer(const CustomContainer& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "CC copy ctor" << endl;
    }

    ~CustomContainer() {
        if (data) {
            delete[] data;
        }
        cout << "CC dctor" << endl;
    }

    void print() const {
        cout << "CC print" << endl;
    }
};


class CustomContainerM {
private:
    size_t size;
    int* data;

public:
    CustomContainerM(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "CCM ctor" << endl;
    }

    CustomContainerM(const CustomContainerM& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "CCM copy ctor" << endl;
    }

    CustomContainerM(CustomContainerM&& other) noexcept:
        size(other.size) {
        data = other.data;
        other.data = nullptr;
        cout << "CCM move ctor" << endl;
    }

    CustomContainerM& operator=(const CustomContainerM& other){
        if (this != &other) {
            size = other.size;
            int* new_data = new int[size];
            std::copy(other.data, other.data + other.size, new_data);
            delete[] data;
            data = new_data;
            cout << "CCM copy =" << endl;
        }
        return *this;
    }

    CustomContainerM& operator=(CustomContainerM&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    ~CustomContainerM() {
        if (data) {
            delete[] data;
        }
        cout << "CCM dctor" << endl;
    }

    void print() const {
        cout << "CCM print" << endl;
    }
};

CustomContainer create_cc_rvo() {
    cout << "create CC RVO" << endl;
    return CustomContainer(100);
}

CustomContainer create_cc_nrvo() {
    cout << "create CC NRVO" << endl;
    CustomContainer obj = CustomContainer(100);
    return obj;
}

CustomContainer create_cc_move() {
    cout << "create CC MOVE" << endl;
    return std::move(CustomContainer(100));
}

CustomContainerM create_ccm_rvo() {
    cout << "create CC RVO" << endl;
    return CustomContainerM(100);
}

CustomContainerM create_ccm_nrvo() {
    cout << "create CC NRVO" << endl;
    CustomContainerM obj = CustomContainerM(100);
    return obj;
}

CustomContainerM create_ccm_move() {
    cout << "create CC MOVE" << endl;
    return std::move(CustomContainerM(100));
}

void recieve_cc(CustomContainer value) {
    cout << "print_cc as value" << endl;
    value.print();
}

void recieve_cc(CustomContainer& value) {
    cout << "print_cc as ref" << endl;
    value.print();
}

void recieve_cc(CustomContainer&& value) {
    cout << "print_cc as rref" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM value) {
    cout << "print_cc as value" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM& value) {
    cout << "print_cc as ref" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM&& value) {
    cout << "print_cc as rref" << endl;
    value.print();
}

int main() {
	// ...
}
```

```cpp
int main() {
    CustomContainer cc0 = CustomContainer(100);
    return 0;
}
```

```
CC ctor
CC dctor
```

```cpp
int main() {
    CustomContainer cc1{CustomContainer(100)};
    return 0;
}
```

```
CC ctor
CC dctor
```

```cpp
int main() {
    CustomContainer cc2 {create_cc_rvo()};
    return 0;
}
```

```
create CC RVO
CC ctor
CC dctor
```

```cpp
int main() {
    CustomContainer cc3 {create_cc_nrvo()};
    return 0;
}
```

```
create CC NRVO
CC ctor
CC dctor
```

```cpp
int main() {
    CustomContainer cc4 {create_cc_move()};
    return 0;
}
```

```
create CC MOVE
CC ctor
CC copy ctor
CC dctor
CC dctor
```

```cpp
int main() {
    CustomContainerM cc = CustomContainerM(100);
    return 0;
}
```

```
CCM ctor
CCM dctor
```

```cpp
int main() {
    CustomContainerM cc{CustomContainerM(100)};
    return 0;
}
```

```
CCM ctor
CCM dctor
```

```cpp
int main() {
    CustomContainerM cc2 {create_ccm_rvo()};
    return 0;
}
```

```
create CC RVO
CCM ctor
CCM dctor
```

```cpp
int main() {
    CustomContainerM cc {create_ccm_nrvo()};
    return 0;
}
```

```
create CC NRVO
CCM ctor
CCM dctor
```

```cpp
int main() {
    CustomContainerM cc {create_ccm_move()};
    return 0;
}
```

```
create CC MOVE
CCM ctor
CCM move ctor
CCM dctor
CCM dctor
```
