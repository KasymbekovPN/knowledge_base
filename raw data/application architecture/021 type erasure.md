---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            
	        "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        },        
        {            
	        "name": "release",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release"  
        }  
    ],    
    "testPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "output": {  
                "outputOnFailure": true  
            }  
        }    
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.40)  
project(abs_temp CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// Type erasure: полиморфизм без общего базового класса. Три уровня -  
// std::function (стирает вызываемые объекты), std::any (стирает вообще  
// любой тип), и самодельная обёртка AnyDrawable (Concept/Model idiom -  
// то, как std::function устроен внутри).  
  
#include <any>  
#include <functional>  
#include <iostream>  
#include <format>  
#include <memory>  
#include <string>  
#include <vector>  
  
// ============================================================  
// 1. std::function - стирает любой вызываемый объект с нужной сигнатурой  
// ============================================================  
double add(const double a, const double b) { return a + b; }  
  
struct Multiplier {  
    double factor;  
    double operator()(const double a, const double b) const { return a * b * factor; }  
};  
  
void demoFunction() {  
    std::cout << "--- std::function ---\n";  
  
    const double CAPTURED{10.0};  
    std::vector<std::function<double(double, double)>> ops;  
    ops.push_back(add);  
    ops.push_back([](const double a, const double b) { return a - b; });  
    ops.push_back([CAPTURED](const double a, const double b) { return a + b + CAPTURED; });  
    ops.push_back(Multiplier{2.0});  
  
    // add, лямбды и Multiplier не имеют НИ ОДНОГО общего предка - но все  
    // лежат в одном std::vector<std::function<...>> и вызываются одинаково.    
    for (const auto& op : ops)  
        std::cout << std::format("result: {}\n", op(3, 4));  
}  
  
// ============================================================  
// 2. std::any - стирает вообще любой тип, не только вызываемые объекты  
// ============================================================  
void demoAny() {  
    std::cout << "\n--- std::any ---\n";  
  
    std::vector<std::any> bag;  
    bag.push_back(42);  
    bag.push_back(std::string("hello"));  
    bag.push_back(3.14);  
  
    for (const auto& v: bag) {  
        if (auto* i = std::any_cast<int>(&v)) {  
            std::cout << std::format("int: {}\n", *i);  
        } else if (auto* s = std::any_cast<std::string>(&v)) {  
            std::cout << std::format("string: {}\n", *s);  
        } else if (auto* d = std::any_cast<double>(&v)) {  
            std::cout << std::format("double: {}\n", *d);  
        }    
    }  

    try {  
        std::any_cast<int>(bag[1]);  
    } catch (const std::bad_any_cast& e) {  
        std::cerr << std::format("caught bad_any_cast: {}\n", e.what());  
    }
}  

template <typename T>  
concept Drawable = requires(T value) { { value.draw() }; };

// ============================================================  
// 3. Самодельная обёртка (Concept/Model idiom) - то, как std::function  
// устроен внутри, только под конкретную нужную нам операцию draw().  
// ============================================================  
class AnyDrawable {  
public:  
    // Шаблонный конструктор - принимает ЛЮБОЙ тип T, у которого есть  
    // метод draw(). Никакого общего интерфейса от T не требуется -    // duck typing, проверяемый на этапе компиляции внутри Model<T>.    
    template <typename T>  
    AnyDrawable(T obj): self_{std::make_unique<Model<T>>(std::move(obj))} {}  
  
    // unique_ptr<Concept> сам не копируется - копирование реализуем  
    // через виртуальный clone().    
    AnyDrawable(const AnyDrawable& other) : self_{other.self_->clone()} {}  
    AnyDrawable& operator=(const AnyDrawable& other) {  
        self_ = other.self_->clone();  
        return *this;  
    }  
    AnyDrawable(AnyDrawable&& other) noexcept = default;  
    AnyDrawable& operator=(AnyDrawable&& other) noexcept = default;  
  
    void draw() const { self_->draw(); }  
  
private:  
    // Concept - приватный интерфейс, о существовании которого T ничего  
    // не знает и никогда не узнает.    
    struct Concept {  
        virtual ~Concept() = default;  
        virtual void draw() const = 0;  
        virtual std::unique_ptr<Concept> clone() const = 0;  
    };  
    
    // Model<T> - единственное место, которое знает и про Concept, и про T.  
    // Внутри всё равно самый обычный виртуальный вызов - type erasure не    // отменяет стоимость диспетчеризации, она просто прячет её от T.    
    template<typename T>  
    struct Model : Concept {  
        explicit Model(T obj): obj_(std::move(obj)) {}  
        void draw() const override { obj_.draw(); }  
        std::unique_ptr<Concept> clone() const override {  
            return std::make_unique<Model<T>>(obj_);  
        }  
        T obj_;  
    };  
    std::unique_ptr<Concept> self_;  
};  
  
// Три никак не связанных типа - ни общего предка, ни общего интерфейса,  
// ни малейшего представления друг о друге.  
struct Circle {  
    double r;  
    void draw() const {  
        std::cout << std::format("Circle {}\n", r);  
    }
};  
  
struct SquareShape {  
    double side;  
    void draw() const {  
        std::cout << std::format("SquareShape {}\n", side);  
    }
};  
  
// Представим, что это тип из чужой библиотеки, который мы не можем  
// поменять - у него физически нет и не может появиться общий интерфейс  
// с Circle/SquareShape.  
struct ThirdPartyStar {  
    int points;  
    void draw() const {  
        std::cout << std::format("ThirdPartyStar {}\n", points);  
    }
};  
  
void demoAnyDrawable() {  
    std::cout << "\n--- AnyDrawable (custom type erasure) ---\n";  
  
    std::vector<AnyDrawable> shapes;  
    shapes.push_back(Circle{5.0});  
    shapes.push_back(SquareShape{3.0});  
    shapes.push_back(ThirdPartyStar{5});  
  
    std::cout << "  originals\n";  
    for (const auto& s: shapes) s.draw();  
  
    // Копируем весь вектор целиком - проверяем, что clone() реально  
    // работает и копии независимы от оригиналов.    
    const std::vector<AnyDrawable> copies = shapes;  
    std::cout << "  copies (via clone)\n";  
    for (const auto& s: copies) s.draw();  
}  
  
int main() {  
    demoFunction();  
    demoAny();  
    demoAnyDrawable();  
  
    return 0;  
}
```


`std::function` хранит четыре совершенно разных вызываемых объекта, `std::any` — три разных типа с корректным `bad_any_cast`, а `AnyDrawable` полиморфно хранит и корректно копирует три типа без единого общего предка.

**Идея type erasure**

Классический полиморфизм требует, чтобы конкретный тип заранее опознал себя как участника иерархии — унаследовался от интерфейса, реализовал виртуальные методы, знал о существовании базового класса. Type erasure переворачивает это: пишется обёртка, которая сама, изнутри, умеет работать с любым типом, удовлетворяющим неявному контракту ("умеет `draw()`", "вызывается с такой сигнатурой") — а сам тип T ни сном ни духом не подозревает о существовании этой обёртки. `Circle`, `SquareShape`, `ThirdPartyStar` в примере не имеют ни одного общего предка, `ThirdPartyStar` вообще представляет собой тип из гипотетической чужой библиотеки, который физически нельзя переписать под общий интерфейс — а `AnyDrawable` заставляет их вести себя полиморфно как единый `std::vector<AnyDrawable>`.

**Как это устроено внутри (Concept/Model idiom)**

`AnyDrawable` содержит приватный абстрактный `Concept` с виртуальным `draw()`/`clone()` — и это самая обычная, классическая виртуальная иерархия, только полностью спрятанная внутри обёртки. Шаблонный `Model<T>` — единственное место во всей программе, которое знает и про `Concept`, и про конкретный `T`: он реализует `Concept::draw()`, просто вызывая `obj_.draw()` — то есть требование "у T есть метод `draw()`" проверяется компилятором в момент инстанцирования `Model<T>`, а не декларируется явным наследованием. Шаблонный конструктор `AnyDrawable(T obj)` инстанцирует нужный `Model<T>` под капотом и заворачивает его в `unique_ptr<Concept>` — снаружи виден только `AnyDrawable`, единый конкретный (не шаблонный!) тип, который можно положить в обычный `std::vector`.

Важная честная деталь: type erasure не отменяет цену диспетчеризации — внутри `Model<T>::draw()` всё равно самый обычный виртуальный вызов через vtable `Concept`, ровно та же механика и та же стоимость, что обсуждали в теме "абстрактные классы vs шаблоны". Плюс здесь добавляется ещё и обязательная эвристика владения — `AnyDrawable` вручную реализует copy через `clone()` (виртуальный метод, каждый `Model<T>` знает, как скопировать именно свой `T`), потому что `unique_ptr` сам по себе не копируется. Тест подтвердил, что это реально работает: копия всего `std::vector<AnyDrawable>` прошла без единой ошибки, каждый элемент корректно клонировался через собственный `Model<T>::clone()`.

**`std::function` — та же идея, но готовая и заточенная под вызываемые объекты**

`std::function<double(double, double)>` в примере хранит свободную функцию, лямбду без захвата, лямбду с захватом (`captured`) и функтор `Multiplier` — четыре совершенно разных по типу и размеру сущности, объединённых только совпадающей сигнатурой вызова. Внутри `std::function` устроен ровно как наш `AnyDrawable` — приватный "Concept" под invoke/copy/move/destroy плюс шаблонная "Model" под конкретный тип callable'а — с одной практической доработкой, которой у нас нет: small buffer optimization — небольшие вызываемые объекты (обычно до размера одного-двух указателей) хранятся прямо внутри `std::function` без отдельной аллокации в куче, и только более крупные (например, лямбда с большим захватом) уходят на кучу. Это стоит знать, если `std::function` используется в горячем пути — не каждое присваивание обязательно означает `new`.

**`std::any` — та же машинерия, только без фиксированной "операции"**

`std::any` не привязан ни к какой конкретной сигнатуре (в отличие от `std::function`, заточенного под вызов) — стирает произвольный копируемый тип целиком. Обратная сторона такой универсальности — типобезопасность отодвигается на этап **чтения**, а не хранения: `std::any_cast<T>` требует точного совпадения типа, и промах — это либо `nullptr` (для указательной формы `any_cast<T>(&v)`, как в первом цикле демо), либо исключение `std::bad_any_cast` (для ссылочной/по значению формы) — тест это подтвердил, попытка достать `int` из `any`, реально хранящего `std::string`, поймана штатным `catch`.

**Когда выбирать что**

Классические абстрактные интерфейсы (`IPrinter`/`IPlugin` из прошлых примеров) — когда ты владеешь исходным кодом типов и осознанно проектируешь иерархию способностей заранее, особенно на ABI-границе (interface там должен быть явным контрактом, а не неявным duck typing). Type erasure — когда типы либо чужие и их нельзя менять, либо когда хочется избежать навязывания всем участникам обязательного наследования и указательной семантики ради одной узкой операции (как `std::function` для колбэков) — цена за это гибкость по значению вместо ссылки, скрытая аллокация в куче (кроме случаев с SBO) и объективно более сложный для чтения код обёртки (хотя пользователю обёртки, в отличие от автора, всё это не видно вообще).
