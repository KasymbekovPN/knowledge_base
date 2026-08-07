---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

Четыре минимальных примера, показывающих, как `always_inline`, `noinline`, `pure` и `const` (GCC/Clang `__attribute__`) реально меняют генерируемый ассемблер. Все команды — обычный g++, то же самое можно вставить в Compiler Explorer (godbolt.org), выбрав x86-64 gcc/clang и указав те же флаги оптимизации в поле опций компилятора.

## 01 — always_inline работает даже на -O0

```bash
g++ -O0 -S -o - 01_always_inline/inline_demo.cpp | c++filt
```

Смотреть на `callAlwaysInline(int)`: тела `call` нет вообще, сложение выполняется прямо в месте вызова, хотя весь остальной код не оптимизирован. Для сравнения — `callPlain(int)` в том же выводе содержит настоящий `call`.

## 02 — noinline не даёт заинлайнить, даже если компилятор хочет

```bash
g++ -O2 -S -o - 02_noinline/noinline_demo.cpp | c++filt
```

`callNoinline(int)` — GCC превращает вызов в хвостовой `jmp` (не `call`+`ret`, но и не инлайнинг: тело функции не продублировано). `callPlain2(int)` (без атрибута) на том же `-O2` полностью заинлайнен и свёрнут в одну инструкцию `leal 10(%rdi), %eax`.

## 03 — pure/const объединяют повторные вызовы (CSE)

Обязательно раздельная компиляция: если `caller.cpp` увидит тело функций из `funcs.cpp`, GCC сам выведет отсутствие побочных эффектов, и разница исчезнет. Компилируем и смотрим ассемблер только для caller.cpp — тела из funcs.cpp ему не видны, только объявления с атрибутами:

```bash
g++ -O2 -S -o - 03_pure_const_cse/caller.cpp | c++filt
```

`testNormal` — два `call normalFunc`. `testPure` и `testConst` — один `call`, затем `addl %eax, %eax` (результат удвоен без повторного вызова).

Собрать и запустить целиком (оба файла нужны для линковки):

```bash
g++ -O2 -o cse_demo 03_pure_const_cse/funcs.cpp 03_pure_const_cse/caller.cpp \
    -x c++ - <<< '
extern "C" int printf(const char*, ...);
int testNormal(int); int testPure(int); int testConst(int);
int main() { printf("%d %d %d\n", testNormal(5), testPure(5), testConst(5)); }
'
./cse_demo   # 20 20 20 — результат одинаковый, разница только в кодогенерации
```

## 04 — pure/const в цикле с изменяемой глобальной переменной

```bash
g++ -O2 -S -o - 04_pure_const_loop/caller.cpp | c++filt
```

В `loopPure` запись в `global` происходит перед КАЖДЫМ вызовом `pureRead` (функция вправе её прочитать). В `loopConst` GCC доказывает, что промежуточные записи в `global` внутри цикла мертвы (`constCompute` их точно не видит) и переносит единственную реальную запись на момент **после** цикла — сам вызов `constCompute` при этом всё равно остаётся внутри цикла на каждой итерации (полного выноса вызова за пределы цикла в этой версии GCC не происходит, хотя теоретически мог бы).

Собрать и запустить (проверено, даёт 25 и 30):

```bash
g++ -O2 -o loop_demo 04_pure_const_loop/funcs.cpp 04_pure_const_loop/caller.cpp \
    -x c++ - <<< '
extern "C" int printf(const char*, ...);
int loopPure(int, int); int loopConst(int, int);
int main() { printf("loopPure(3,5)=%d loopConst(3,5)=%d\n", loopPure(3, 5), loopConst(3, 5)); }
'
./loop_demo
```
