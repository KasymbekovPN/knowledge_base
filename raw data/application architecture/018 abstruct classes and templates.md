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
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
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
// Runtime polymorphism (виртуальные функции) vs compile-time polymorphism  
// (шаблоны) - измеряем реальную разницу, а не полагаемся на общие слова.  
//  
// Важная оговорка про честность сравнения: virtualShapes чередует Circle/  
// Square, чтобы вызов через vtable был РЕАЛЬНО полиморфным (megamorphic  
// call site) - иначе компилятор мог бы догадаться о типе и девиртуализировать  
// сам. staticShapes же вынужденно однородны (все CircleStatic) - именно  
// потому что std::vector<Shape> инстанцируется под ОДИН конкретный тип.  
// Это не баг замера, а иллюстрация самого компромисса: шаблоны не умеют  
// держать разнородную коллекцию, virtual - умеет.  
  
#include <chrono>  
#include <cstdio>  
#include <memory>  
#include <random>  
#include <vector>  
  
// ---------- Вариант 1: runtime polymorphism ----------  
class IShape {  
public:  
    virtual ~IShape() = default;  
    virtual double area() const = 0;  
};  
  
class Circle: public IShape {  
public:  
    explicit Circle(const double r): r_{r} {}  
    double area() const override {  
        return 3.14159265358979 * r_ * r_;  
    }private:  
    double r_{0.0};  
};  
  
class Square: public IShape {  
public:  
    explicit Square(const double s): s_{s} {}  
    double area() const override { return s_ * s_; }  
private:  
    double s_{0.0};  
};  
  
double sumAreasVirtual(const std::vector<std::unique_ptr<IShape>>& shapes) {  
    double total{0.0};  
    for (const auto& s: shapes) total += s->area();  
  
    return total;  
}  
  
// ---------- Вариант 2: compile-time polymorphism ----------  
struct CircleStatic {  
    double r;  
    double area() const { return 3.14159265358979 * r * r; }  
};  
  
struct SquareStatic {  
    double s;  
    double area() const { return s * s; }  
};  
  
template <typename Shape>  
double sumAreasStatic(const std::vector<Shape>& shapes) {  
    double total{0.0};  
    // тип известен на этапе компиляции  
    for (const auto& s: shapes) total += s.area();  
  
    return total;  
}  
  
int main() {  
    constexpr int N{20'000'000};  
  
    std::vector<std::unique_ptr<IShape>> virtualShapes;  
    std::vector<CircleStatic> staticShapes;  
    virtualShapes.reserve(N);  
    staticShapes.reserve(N);  
  
    std::mt19937 rng{42};  
    std::uniform_real_distribution<double> distribution{0.1, 10.0};  
    for (int i{}; i < N; ++i) {  
        const double R{distribution(rng)};  
        if (i % 2 ) {  
            virtualShapes.push_back(std::make_unique<Circle>(R));  
        } else {  
            virtualShapes.push_back(std::make_unique<Square>(R));  
        }        
        staticShapes.push_back(CircleStatic{R});  
    }  
    const auto T0{std::chrono::steady_clock::now()};  
    const double TOTAL_VIRTUAL{sumAreasVirtual(virtualShapes)};  
    const auto T1{std::chrono::steady_clock::now()};  
    const double TOTAL_STATIC{sumAreasStatic(staticShapes)};  
    const auto T2{std::chrono::steady_clock::now()};  
  
    const double virtual_ms{std::chrono::duration<double, std::milli>(T1 - T0).count()};  
    const double static_ms{std::chrono::duration<double, std::milli>(T2 - T1).count()};  
  
    std::printf("virtual dispatch: %8.2f ms  (sum=%.3f)\n", virtual_ms, TOTAL_VIRTUAL);  
    std::printf("static  dispatch: %8.2f ms  (sum=%.3f)\n", static_ms, TOTAL_STATIC);  
    std::printf("speedup:          %8.2fx\n", virtual_ms / static_ms);  
  
    return 0;  
}
```

### debug, windows
```
virtual dispatch:   586.23 ms  (sum=1394605483.180)  
static  dispatch:   184.31 ms  (sum=2115658697.809)  
speedup:              3.18x
```

### release, windows
```
virtual dispatch:    70.35 ms  (sum=1394605483.180)  
static  dispatch:    12.02 ms  (sum=2115658697.809)  
speedup:              5.85x
```

Дизассемблирование подтверждает всё буквально. `sumAreasVirtual` компилируется в:

```
mov  (%rbx),%rdi      ; забрать указатель на объект из unique_ptr
mov  (%rdi),%rax       ; забрать vptr — указатель на vtable объекта
call *0x10(%rax)       ; КОСВЕННЫЙ вызов — функция по смещению 0x10 в vtable
```

Это ровно механизм виртуального вызова: два дополнительных чтения из памяти (объект → vptr → адрес функции) плюс косвенный переход, а `Circle::area()`/`Square::area()` остаются отдельными нескомпилированными в место вызова функциями. `sumAreasStatic`, наоборот, компилятор инлайнил целиком прямо в `main` — там просто `mulsd`/`addsd` в цикле, ни одного `call`.**Как работает виртуальный вызов механически**

Класс с виртуальными функциями получает скрытое поле — указатель на vtable (`vptr`), добавляемое компилятором в начало объекта (это, кстати, ещё одна причина, почему `sizeof` полиморфного класса больше, чем у обычного — ещё один параллельный пример вмешательства компилятора в layout, о котором говорили в контексте pImpl). Вызов `shape->area()` — это не прямой переход к коду функции, а: прочитать `vptr` из объекта, прочитать из vtable по фиксированному смещению адрес нужной функции, перейти по нему косвенно. Именно это дизассемблирование и показало: `mov (%rdi),%rax` → `call *0x10(%rax)`.

**Как работают шаблоны**

Компилятор инстанцирует отдельную копию кода функции под каждый конкретный тип на этапе компиляции — вызов `s.area()` внутри `sumAreasStatic<CircleStatic>` компилятор статически знает, что это `CircleStatic::area()`, и может не просто вызвать её напрямую, а заинлайнить целиком, что здесь и произошло — GCC растворил весь вызов в паре `mulsd`/`addsd` прямо в теле цикла.

Качественно код совершенно разный (косвенный вызов + невозможность инлайнить vs полностью заинлайненная арифметика). Причина — в этом конкретном тесте узкое место не сам косвенный переход (современные процессоры неплохо предсказывают даже мегаморфные call site'ы, если паттерн типов достаточно регулярный, как здесь чередование Circle/Square), а то, что 20 миллионов `unique_ptr<IShape>` — это 20 миллионов отдельных выделений в куче, разбросанных по памяти: цикл упирается в промахи кэша при чтении `vptr` и данных объекта, а не в стоимость самого `call`. Это прямая иллюстрация темы ECS, которую разбирали раньше: реальная цена "ООП с указателями на кучу" чаще всего не в виртуальном вызове как инструкции, а в паттерне доступа к памяти, который он за собой тянет (рассеянные аллокации вместо плотного массива).

Более честная цена виртуального вызова — не в тактах на сам переход, а в **потерянных возможностях оптимизации**: компилятор не может заинлайнить `area()` через виртуальную границу (в общем случае — без devirtualization, который срабатывает только когда компилятор статически доказывает единственный возможный конкретный тип, например через `final` на классе или LTO), а значит не может дальше протащить константы, объединить вычисления, векторизовать цикл — то, что случилось со статической версией бесплатно, для виртуальной невозможно в принципе, даже если сам косвенный переход стоил бы условно "0 тактов".

**Когда что выбирать**

Шаблоны — когда набор типов известен на этапе компиляции и не меняется в рантайме, когда нужна максимальная производительность в горячем пути (STL, `std::sort`, числовые библиотеки — весь `<algorithm>` построен на этом принципе), когда сущностей действительно однородный набор (тот же `ComponentStorage<T>` из ECS-примера — компонентные массивы гомогенны по определению).

Абстрактные классы — когда конкретный тип определяется в рантайме, а не в момент компиляции: пользовательский ввод, конфиг, загруженный плагин (наш пример из `plugin_demo` — `IPlugin` обязан быть виртуальным, потому что до `dlopen` компилятор физически не знает, какой класс будет создан; шаблон здесь принципиально неприменим — инстанцировать шаблон под неизвестный на этапе компиляции тип невозможно). Также виртуальные функции нужны, когда требуется **разнородная** коллекция в одном контейнере (`std::vector<std::unique_ptr<IShape>>` с разными конкретными фигурами) — шаблонный контейнер всегда инстанцируется под один конкретный тип, гетерогенность ему insurmountable в принципе. И ещё виртуальные функции дают стабильную точку ABI-границы между модулями — та же причина, по которой `IPaymentGateway`/`IPlugin` в наших прошлых примерах жили на границах библиотек, а не были шаблонами: шаблонный код должен целиком лежать в заголовке (что ломает границу компиляции, о которой говорили в теме pImpl/модульности), виртуальный интерфейс — нет, реализация может быть полностью скрыта в `.cpp`/`.so`.
