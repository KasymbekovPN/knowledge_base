---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__declspec(novtable)` — оптимизация MSVC для абстрактных базовых классов (чистые интерфейсы, только с виртуальными функциями), которая убирает инициализацию vtable-указателя в конструкторе и деструкторе этого класса. Обычно каждый конструктор/деструктор класса с виртуальными функциями обязан установить указатель на _свою_ vtable (даже для промежуточных базовых классов в иерархии — это нужно, чтобы виртуальные вызовы работали корректно, если вызвать виртуальную функцию прямо из конструктора базового класса). Если класс никогда не создаётся напрямую — только как интерфейс, от которого наследуются, — эта инициализация бесполезна, но всё равно генерируется компилятором и слегка раздувает код.

```cpp
class __declspec(novtable) IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual const char* name() const = 0;
    virtual void execute() const = 0;
};

class ConcretePlugin : public IPlugin {
public:
    const char* name() const override { return "Concrete"; }
    void execute() const override { /* ... */ }
};
```

Здесь `IPlugin` — ровно такой интерфейс, как в демо-проекте с плагинами выше: класс с чисто виртуальными функциями, который сам по себе никогда не инстанцируется, только как база для `ConcretePlugin`.

Критически важное ограничение: `novtable` даёт компилятору право **не** устанавливать vtable-указатель в конструкторе/деструкторе именно этого класса. Если класс, помеченный `novtable`, всё-таки создаётся напрямую (не должен быть чисто абстрактным — например, у него есть невиртуальные конструкторы/поля, или кто-то по ошибке создаёт временный объект такого типа), либо если внутри его собственного конструктора/деструктора вызывается виртуальная функция — это UB: vtable-указатель может указывать в никуда или на vtable производного класса раньше времени. Поэтому `novtable` можно ставить только на классы, у которых:

- все конструкторы `protected` или класс чисто абстрактный (есть хотя бы один pure virtual, экземпляр напрямую создать нельзя);
- ни конструктор, ни деструктор не вызывают виртуальные функции.

Практическая польза — небольшое уменьшение размера кода и чуть более быстрое построение объектов в больших иерархиях с частым созданием/уничтожением объектов через интерфейсы (COM-подобные архитектуры, где `novtable` исторически особенно распространён — сам COM в Windows SDK активно использует этот приём для интерфейсов вида `IUnknown`). Стандартного `[[...]]`-аналога и GCC/Clang-эквивалента нет — это специфическая для MSVC ABI-оптимизация, в кроссплатформенном коде обычно просто оборачивают в макрос, разворачивающийся в пустоту на других компиляторах:

```cpp
#if defined(_MSC_VER)
  #define NOVTABLE __declspec(novtable)
#else
  #define NOVTABLE
#endif

class NOVTABLE IPlugin { /* ... */ };
```

### Пример
```cpp
#include <format>  
#include <iostream>  
  
#if defined(_MSC_VER)  
    #define NOVTABLE __declspec(novtable)  
#else  
    #define NOVTABLE  
#endif  
  
namespace {  
    class NOVTABLE IPlugin {  
    public:  
        virtual ~IPlugin() = default;  
        virtual const char* name() const = 0;  
        virtual void execute() const = 0;  
    };  
    class ConcretePlugin: public IPlugin {  
    public:  
        const char* name() const override { return "ConcretePlugin"; }  
        void execute() const override { std::cout << "[OK]\n"; }  
    };}  
  
int main() {  
    const ConcretePlugin plugin;  
    std::cout << std::format("name: {}\n", plugin.name());  
    plugin.execute();  
  
    return 0;  
}
```
