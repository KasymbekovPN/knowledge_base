---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[no_unique_address]]` (C++20) — атрибут на нестатическом поле класса, разрешающий компилятору не резервировать под это поле отдельный адрес, если оно пустое (empty class, как `struct Empty {}` без данных) — то есть позволяет "наложить" его на память соседних полей, устраняя обычное padding-раздутие от EBO (empty base optimization), но применённое к полям-членам, а не только к базовым классам. Классический кейс — кастомные аллокаторы, компараторы, deleter'ы в контейнерах: часто это stateless-объекты, и без атрибута каждое такое поле съедало бы минимум 1 байт (плюс выравнивание).

```cpp
#include <iostream>  
  
namespace {  
    struct Empty {};  
  
    struct WithoutAttr {  
        Empty e;  
        int x;  
    };  
    struct WithAttr {  
        // [[no_unique_address]] Empty e;  
        [[msvc::no_unique_address]] Empty e;  
        int x;  
    };
}  
  
int main() {  
    std::cout << "WithoutAttr: " << sizeof(WithoutAttr) << std::endl;  
    std::cout << "WithAttr: " << sizeof(WithAttr) << std::endl;  
  
    return 0;  
}
```

Реальный пример из стандартной библиотеки — `std::unique_ptr<T, Deleter>` часто хранит `Deleter` как поле; если deleter пустой (например, `std::default_delete`), с этим атрибутом он не увеличивает размер `unique_ptr`. Нюанс: MSVC до недавнего времени игнорировал атрибут по умолчанию из-за ABI-совместимости (нужен был `/Zc:__cplusplus` и всё равно не всегда работало) — там появился отдельный `[[msvc::no_unique_address]]`. В C++23 гарантии атрибута усилены (обязателен zero-size для действительно пустых классов, если не единственное поле того же типа подряд).
