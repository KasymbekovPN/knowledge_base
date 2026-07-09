---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]


## 1. Как GCC на самом деле называет coroutine frame в символах
Вот ключевая деталь — GCC компилирует **одну** твою функцию `fibonacci(int)` в **три** отдельных символа:

|Символ|Роль|
|---|---|
|`fibonacci(int)`|**Ramp function** — начальная точка, выделяет coroutine frame в куче, вызывается один раз при первом вызове|
|`fibonacci(...).Frame*) [clone .actor]`|**Actor function** — тело корутины, вызывается при **каждом** `resume()`|
|`fibonacci(...).Frame*) [clone .destroy]`|**Destroy function** — вызывается при уничтожении frame (`coro.destroy()` или после `final_suspend`)|

Именно поэтому в наших ранних экспериментах backtrace показывал `fibonacci(_Z9fibonaccii.Frame *)` вместо привычного `fibonacci(int)` — GDB на самом деле стоит внутри **actor**-функции, а не в исходной сигнатуре.

## 2. Читаем состояние `promise_type` напрямую через frame pointer`*frame_ptr` дампит **весь** compiler-generated coroutine frame разом — и вот наш `promise_type` прямо внутри: `_Coro_promise = {current_value = 0}`. 
Обрати внимание на суффиксы у локальных переменных (`a_1_2`, `b_1_2`, `i_2_3`, `next_3_4`) — GCC добавляет числовые метки для различения версий переменной в разных "состояниях" state machine (переменная `a` физически одна в памяти, но компилятор помечает её версию на конкретном этапе выполнения).

## Прямой доступ к конкретному полю `promise_type`
Прямо видно эволюцию состояния корутины между `resume()`: `current_value` изменилось `0 → 1` после `continue` — то есть мы читаем **реальное, живое** состояние `promise_type` внутри heap-allocated frame, а не какую-то тень или копию.

Важный практический момент: **имя `frame_ptr`** — не гарантированная константа. Оно зависит от того, как GCC назвал параметр actor-функции в конкретной версии компилятора. Более надёжный способ — через `_Coro_self_handle`, если ты стоишь не внутри actor-функции, а снаружи (в вызывающем коде):Интересный побочный момент — `gen.coro.promise()` — это **настоящий вызов функции** через inferior call, и в отличие от LLDB (где мы видели проблемы с `operator*`/`.get()` на `unique_ptr`), в GDB это сработало без проблем.

## 3. Проблема с оптимизированными фреймами (`-Og`/`-O2`)
На `-Og` в этом случае поля остались нетронутыми — важное напоминание, что `-Og` **специально спроектирован** не ломать отладку. Возьмём настоящую агрессивную оптимизацию `-O2`, чтобы увидеть реальную деградацию:Отличная демонстрация того самого "проблемы с оптимизированными фреймами": на `-O2` breakpoint на строке 46 **не туда попал** — вместо тела корутины (actor-функция) он приземлился на строку 42 (`Generator<long long> fibonacci(int count) {` — начало **ramp**-функции). `frame_ptr` вообще не существует в этом контексте (`No symbol "frame_ptr" in current context`) — компилятор настолько всё перекроил, что привычный параметр actor-функции пропал из этой точки зрения на код.Обрати внимание на новый символ: **`[clone .actor] [clone .cold]`** — GCC на `-O2` включает **hot/cold code splitting**: редко исполняемый код (например, exception-обработка) выносится в отдельную "холодную" секцию для лучшей работы icache. Это ещё один источник путаницы — теперь у тебя **два** физических куска actor-функции.

## Надёжный способ ставить breakpoint на оптимизированной корутине — по символу, а не по строке
Прорыв — обычный **`break fibonacci`** (без уточнения сигнатуры) дал **4 locations** сразу (ramp + hot actor + cold actor + destroy), и backtrace на `Breakpoint 1.4` показал что-то очень поучительное:

```
#0  fibonacci (frame_ptr=0x55555556b2b0) at coro_demo.cpp:42   ← actor
#1  fibonacci (count=count@entry=10) at coro_demo.cpp:42        ← ramp
#2  main () at coro_demo.cpp:53
```

Это подтверждает деталь ABI корутин, о которой редко пишут: **ramp-функция всегда безусловно вызывает actor один раз** (чтобы дойти до первой точки suspend), даже если `initial_suspend()` возвращает `suspend_always` — просто именно **в этой первой actor-инстанции** выполнение реально остановится на suspend point. `frame_ptr` существует именно в **этом**, самом глубоком фрейме (`#0`), хотя минутой ранее в этой же сессии, на breakpoint по `file:line`, GDB "терял" его — потому что попадал не в тот фрейм.

**Самый надёжный breakpoint на оптимизированной корутине — по короткому имени функции без уточнений** (`break fibonacci`), а не по конкретной строке — GDB сам покажет все N locations, и ты увидишь через `info breakpoints`, в каком фрейме реально нужный `frame_ptr`.

### Пример

### main.cpp
```cpp
#include <coroutine>  
#include <optional>  
#include <iostream>  
#include <format>  
  
// Простой Generator<T> — типичный пример из твоих заметок по корутинам  
template<typename T>  
struct Generator {  
    struct promise_type {  
        T current_value;  
  
        Generator get_return_object() {  
            return Generator{  
                std::coroutine_handle<promise_type>::from_promise(*this)  
            };        }        std::suspend_always initial_suspend() { return {}; }  
        std::suspend_always final_suspend() noexcept { return {}; }  
        std::suspend_always yield_value(T _value) {  
            current_value = _value;  
            return {};  
        }        void return_void() {}  
        void unhandled_exception() { std::terminate(); }  
    };  
    using handle_type = std::coroutine_handle<promise_type>;  
    handle_type coro;  
  
    explicit Generator(handle_type _coro) : coro(_coro) {}  
    ~Generator() { if (coro) coro.destroy(); }  
    Generator(Generator&&) = delete;  
    Generator(const Generator&) = delete;  
  
    bool next() {  
        coro.resume();  
        return !coro.done();  
    }  
    T value() const { return coro.promise().current_value; }  
};  
  
// Генератор чисел Фибоначчи — суспендится на каждом yield  
Generator<long long> fibonacci(int _count) {  
    long long a{0}, b{1};  
    for (int i{}; i < _count; ++i) {  
        co_yield a;  
        const long long next{a + b};  
        a = b;  
        b = next;  
    }}  
  
int main() {  
    auto gen = fibonacci(10);  
    long long sum{0};  
    while (gen.next()) {  
        long long v{gen.value()};  
        std::cout << std::format("{} ", v);  
        sum += v;  
    }    std::cout << std::format("\nSum: {}", sum);  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 47  
run  
breakpoint list  
thread backtrace  
frame variable  
print __promise  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:47  
run  
bt  
info locals  
print __promise  
  
 */
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(proj LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)
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
    ]
}
```

### vcpkg.json
```json
{  
    "name": "demo",  
    "version": "0.1.0",  
    "dependencies": []  
}
```
