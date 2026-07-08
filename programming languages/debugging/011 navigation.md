---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Step / Next / Finish / Until — тонкости при инлайнинге

## Базовые команды и их аналоги

|Действие|GDB|LLDB|Что делает|
|---|---|---|---|
|Step into|`step` / `s`|`step` / `s` (или `thread step-in`)|Заходит **внутрь** вызываемой функции|
|Step over|`next` / `n`|`next` / `n` (или `thread step-over`)|Выполняет строку **целиком**, не заходя внутрь вызовов|
|Step out|`finish`|`finish` (или `thread step-out`)|Доводит выполнение до **конца текущей функции** и возврата к вызывающей|
|Until line|`until <line>`|нет прямого аналога (обходится через breakpoint)|Выполняет до указанной строки **в той же функции**, полезно чтобы проскочить цикл|
|Step инструкция|`stepi` / `si`|`si` (`thread step-inst`)|Шаг на уровне **одной машинной инструкции**|

## Главная тонкость: что происходит, когда функция заинлайнена

Когда компилятор инлайнит функцию (заменяет вызов на подставленный код прямо на месте), **физически вызова функции больше нет** — это просто продолжение той же линейной последовательности инструкций. Отсюда вопрос: что делает `step`, если "функции", в которую нужно "войти", уже не существует как отдельного вызова?

Ответ зависит от того, есть ли DWARF-информация об инлайнинге (`DW_TAG_inlined_subroutine`):

- **Если инфа есть** — отладчик умеет "притворяться", что заходит внутрь инлайненной функции, даже если физически это одна прямая последовательность инструкций
- **Если инфы нет** (или отладчик её игнорирует) — `step` просто проскакивает мимо, как будто там ничего не было (потому что физически там действительно нет "входа")

## Пример

### main.cpp
```cpp
#include <iostream>  
#include <format>  
#include <cstdlib>  
  
// Маленькая функция — идеальный кандидат на инлайнинг компилятором  
inline int square(int x) {  
    int result{x * x}; // <-- сюда попробуем step внутрь  
    return result;  
}  
  
int compute(int a, int b) {  
    int sq_a = square(a); // <-- breakpoint здесь: step vs next  
    int sq_b = square(b);  
    int sum = sq_a + sq_b; // <-- смотрим, куда попали после finish/step  
    return sum;  
}  
  
int main(int argc, char *argv[]) {  
    int x = argc > 1 ? std::atoi(argv[1]) : 3;  
    int y = argc > 2 ? std::atoi(argv[2]) : 4;  
    std::cout << std::format("result = {}\n", compute(x, y));  
  
    return 0;  
}  
  
/*  
  
# GDB  
C:\msys64\clang64\bin\gdb.exe --args .\build\debug\app.exe 6 7  
C:\msys64\clang64\bin\gdb.exe --args .\build\release\app.exe 6 7  
break main.cpp:12  
run  
bt  
step  
  
# LLDB  
lldb -- .\build\debug\app.exe 6 7  
lldb -- .\build\release\app.exe 6 7  
breakpoint set --file main.cpp --line 12  
run  
bt  
step  
  
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
        },
		{
			"name": "release",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O2"  
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

## Практика — соберём пример с инлайн-функцией

## Сценарий 1: `-O0` — square() НЕ заинлайнена, step заходит физически внутрь

На `-O0` всё предсказуемо: `step` физически заходит в `square()` — отдельный фрейм в backtrace (`#0 square`, `#1 compute`, `#2 main`), потому что на этом уровне оптимизации компилятор **не** инлайнит, даже несмотря на `inline` в исходнике (это лишь подсказка компилятору, не гарантия).

## Сценарий 2: `-O2` — square() ЗАИНЛАЙНЕНА

Ещё более показательный результат, чем я ожидал: breakpoint на строке 10 **вообще не сработал** — программа просто выполнилась до конца (`result = 25` — и всё). Проверим, что произошло на самом деле:**Невероятный, но крайне поучительный результат**: `$0x19` (=25 в hex) — компилятор на `-O2` не просто заинлайнил `square()`/`compute()`, а **вычислил весь результат на этапе компиляции** (interprocedural constant propagation), потому что `x=3, y=4` были литералами. От `compute()` и `square()` в машинном коде **не осталось вообще ничего** — GDB прямо говорит: `Line 10 ... contains no code`. Поэтому breakpoint не мог сработать в принципе.

Чтобы увидеть **настоящий** инлайнинг (не constant folding), уберём литералы — возьмём значения из аргументов командной строки:Отлично — теперь `square()` и `compute()` реально заинлайнены **внутрь `main()`** (не выброшены полностью), обе строки указывают на один и тот же адрес `0x1141` внутри `main`. Смотрим, что покажет `step`/`bt`:Феноменальный результат — вот в чём соль инлайнинга и DWARF:

## Что произошло

```
Breakpoint 1 at 0x1141: inline_demo.cpp:11. (2 locations)
```

GDB сразу предупредил: **2 locations** — потому что `square()` вызывается дважды в `compute()`, и обе инстанциации заинлайнены в **разных** местах машинного кода (даже несмотря на общий адрес входа).

```
#0  square (x=<optimized out>) at inline_demo.cpp:13
#1  compute (b=7, a=6) at inline_demo.cpp:12
#2  main (...) at inline_demo.cpp:20
```

Смотри внимательно: это **три логических фрейма**, хотя физически на стеке — **один-единственный** реальный x86 stack frame (`main`'а). GDB **реконструирует виртуальный backtrace** из DWARF-тегов `DW_TAG_inlined_subroutine` — он "притворяется", что есть отдельные вызовы `square`/`compute`, хотя в реальности это линейный код внутри `main`. Это именно та "магия", о которой я говорил в начале раздела.

После `step`:

```
#0  main (argc=<optimized out>, argv=<optimized out>) at inline_demo.cpp:20
```

Backtrace **схлопнулся до одного фрейма** — `step` продвинул выполнение за пределы "виртуальной" инлайн-границы, назад в прямолинейный код `main`, где больше нет иллюзии вложенных вызовов.

## Сравним с LLDB — как он маркирует inline-фреймыВот ключевая разница в **явности** между отладчиками:

**LLDB честно помечает** каждый виртуальный фрейм тегом `[inlined]`:

```
frame #0: main [inlined] compute(...) at inline_demo.cpp:13
frame #1: main(...) at inline_demo.cpp:20
```

Ты сразу видишь: "это `main`, внутри которого логически инлайнена `compute`" — прозрачно и недвусмысленно. GDB же просто рисует отдельные фреймы `#0 square`, `#1 compute`, `#2 main`, **не помечая** их как инлайн — выглядит как настоящий вызов функций, хотя это не так. Это прямое продолжение разницы философий, о которой мы говорили раньше: LLDB стремится к явности и точности, GDB — к привычному "выглядит как обычный backtrace".

## Сводка тонкостей step/next/finish при инлайнинге

1. **`step` на инлайн-границе** — не создаёт новый физический фрейм, просто продолжает выполнение и корректирует **логическую** позицию (строку/функцию) в отображении — реальный `$rsp`/`$rbp` не меняются.
2. **`next` (step over)** на инлайн-вызове — обычно просто проходит его целиком за одну "виртуальную" строку, аналогично обычной функции, разницы в поведении почти нет по сравнению со `step`, если внутри инлайна нет breakpoint'ов.
3. **`finish`** внутри инлайн-фрейма — GDB и LLDB оба умеют "выйти" из виртуального инлайн-фрейма обратно в настоящего "хозяина" (`main`), даже без реального `ret`.
4. **`<optimized out>` для аргументов** — как видно выше (`x=<optimized out>`, `argc=<unavailable>`) — типичное последствие агрессивной оптимизации: сами значения регистров переиспользуются компилятором, поэтому просмотр параметров инлайн-функции часто недоступен.
5. **Множественные locations у одного breakpoint** (`2 locations`) — прямое следствие того, что один и тот же инлайнируемый вызов может физически встречаться в нескольких местах кода (циклы с разверткой, несколько call sites).## Быстрый чеклист для сборки/повтора
