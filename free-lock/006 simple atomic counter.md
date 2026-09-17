
### Реализовать простой atomic counter с разными memory_order и посмотреть разницу в асме (godbolt)


# Разница в асме на x86-64 (Intel syntax, `-O2`)

Собрал прямо здесь через `g++ -S -masm=intel`, но эквивалентно тому, что покажет godbolt.compiler-explorer.com с той же командной строкой — можете вставить туда для интерактивной проверки.

## Код

```cpp
std::atomic<long> counter{0};

void inc_relaxed()   { counter.fetch_add(1, std::memory_order_relaxed); }
long load_acquire()  { return counter.load(std::memory_order_acquire); }
void store_release(long v) { counter.store(v, std::memory_order_release); }
void inc_acq_rel()   { counter.fetch_add(1, std::memory_order_acq_rel); }
void inc_seq_cst()   { counter.fetch_add(1, std::memory_order_seq_cst); }
void store_seq_cst(long v) { counter.store(v, std::memory_order_seq_cst); }
```

[[https://godbolt.org/]]

## Результат -O3

```asm
"main":
        lock add        QWORD PTR "(anonymous namespace)::counter"[rip], 1
        mov     rax, QWORD PTR "(anonymous namespace)::counter"[rip]
        mov     QWORD PTR "(anonymous namespace)::counter"[rip], 21
        lock add        QWORD PTR "(anonymous namespace)::counter"[rip], 1
        lock add        QWORD PTR "(anonymous namespace)::counter"[rip], 1
        mov     eax, 42
        xchg    rax, QWORD PTR "(anonymous namespace)::counter"[rip]
        xor     eax, eax
        ret
```

## Разбор по строчкам — самое важное открытие

**`fetch_add` — вообще не зависит от memory_order на x86.** `relaxed`, `acq_rel` и `seq_cst` версии дают **идентичный** `lock add`. Это прямое следствие того, что `LOCK`-префикс на x86 уже даёт полный барьер сам по себе (аппаратно) — компилятору некуда "ослаблять" инструкцию, `lock add` и так самый дешёвый вариант атомарного RMW, слабее его физически не сделать на этой архитектуре. Отсюда практический вывод: **на x86 выбор memory_order для RMW-операций почти не влияет на производительность**, только на то, что компилятор _разрешает_ переставлять вокруг них на уровне других инструкций (compile-time reordering), но не на саму инструкцию.

**`load(acquire)` компилируется в простой `mov`.** Никакого барьера вообще — на x86 обычная загрузка **уже** имеет acquire-семантику по умолчанию (сильная модель памяти x86-TSO гарантирует, что loads не переупорядочиваются с последующими loads/stores в общем случае). Acquire здесь бесплатен буквально в железе.

**`store(release)` — тоже простой `mov`.** Аналогично: обычная запись на x86 уже имеет release-семантику "из коробки" — store не переупорядочивается с предыдущими loads/stores с точки зрения других ядер (кроме store-load reordering, но это не то, что запрещает release).

**`store(seq_cst)` — вот где реальная разница: `xchg` вместо `mov`.** Это ключевое отличие. `XCHG` с операндом в памяти на x86 **неявно имеет LOCK-префикс** (это единственная инструкция, где lock подразумевается автоматически) — то есть компилятор специально заменил бы дешёвый `mov` на дорогой atomic `xchg`, чтобы дать полный барьер, которого требует seq_cst store (запрет на store-load reordering — единственный тип reordering, который x86-TSO **разрешает** сам по себе, и seq_cst обязан его запретить).

## Практический вывод, применимый к вашему коду

Вернёмся к Treiber stack и spinlock, которые уже разбирали — вот почему выбор `acquire`/`release`/`acq_rel` вместо `seq_cst` там был осознанным, а не просто "чтобы было красивее":

|Операция|acq_rel/acquire/release|seq_cst|Реальная разница на x86|
|---|---|---|---|
|`fetch_add`/CAS (RMW)|`lock ...`|`lock ...`|**Нет разницы** — LOCK-префикс уже полный барьер|
|`load`|`mov`|`mov` + доп. барьер в некоторых случаях*|Обычно нет разницы для чистого load|
|`store`|`mov`|`xchg` (implicit lock)|**Есть разница** — seq_cst store заметно дороже|

*Для чистого `seq_cst` load компилятор иногда всё равно оставляет просто `mov`, потому что x86 и так не переупорядочивает loads относительно других loads — разница в seq_cst проявляется именно на **store**, где нужно специально запретить store-load reordering.

**Итого:** на x86 разница между "правильным" acquire/release и "избыточным" seq_cst реально ощутима только на **store**-операциях (`mov` → `xchg`), а на RMW (fetch_add, CAS) разницы нет вообще. Это объясняет, почему на x86 многиеlock-free библиотеки "по умолчанию используют seq_cst и не парятся" — реальная цена почти нулевая для большинства паттернов, кроме hot-path чистых сторов в очень горячем коде (там `mov` vs `xchg` может дать заметный выигрыш при миллиардах операций в секунду).

## Что стоит проверить на ARM (принципиально другая картина)

На ARM (aarch64) картина совсем другая — там **нет** сильной модели памяти x86-TSO, там всё явно барьерится инструкциями `ldar`/`stlr` (load-acquire/store-release) или `dmb`, и разница между relaxed/acquire/release/seq_cst видна на **каждой** операции, включая load и RMW. Если хотите — можем скомпилировать этот же файл кросс-компилятором под aarch64 (`aarch64-linux-gnu-g++`) и сравнить, насколько сильнее там расходятся варианты — это хорошо иллюстрирует, почему memory_order вообще существует как абстракция, а не просто "синтаксический сахар для x86".
