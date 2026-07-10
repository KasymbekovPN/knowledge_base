
[[raw data/cpp/interview/_|<=]]

# `make_shared` vs `make_unique`

Обе — фабричные функции, создающие объект в куче и оборачивающие его в smart pointer. Но устроены и мотивированы по-разному.

## Общая мотивация (зачем вообще нужны)

### 1. Exception safety

Главная историческая причина. Рассмотри вызов с двумя аргументами:

```cpp
void process(std::shared_ptr<Widget> w, int priority);

process(std::shared_ptr<Widget>(new Widget), computePriority());   // ОПАСНО
```

Компилятор вправе вычислять аргументы в любом порядке. Возможна последовательность:

1. `new Widget` — память выделена, объект создан
2. `computePriority()` — **бросает исключение**
3. `shared_ptr` так и не был сконструирован → **утечка** сырого `Widget`

С фабрикой такой дыры нет — выделение и захват владения происходят внутри одного вызова, атомарно с точки зрения exception safety:

```cpp
process(std::make_shared<Widget>(), computePriority());   // безопасно
```

### 2. Читаемость и отсутствие голого `new`

```cpp
auto w = std::make_unique<Widget>(a, b);   // тип понятен, new не виден
std::unique_ptr<Widget> w(new Widget(a, b));  // дублирование Widget, голый new
```

Правило современного C++: **никаких голых `new`/`delete`** в прикладном коде.

---

## Ключевое различие: количество аллокаций

### `make_unique` — одна аллокация (как и обычный new)

```cpp
auto u = std::make_unique<Widget>(args);
```

`unique_ptr` не имеет control block — только указатель. Поэтому `make_unique` делает ровно **одну** аллокацию (сам объект), ничем не отличаясь по layout'у от `new Widget`. Выигрыш здесь чисто в exception safety и читаемости, а не в производительности.

### `make_shared` — одна аллокация вместо двух

```cpp
auto s = std::make_shared<Widget>(args);
```

`shared_ptr` требует control block. Разница драматична:

```cpp
std::shared_ptr<Widget> s(new Widget);   // ДВЕ аллокации: объект + control block
auto s = std::make_shared<Widget>();     // ОДНА аллокация: объект и блок в одном куске
```

```
new Widget:          make_shared:
┌────────┐           ┌──────────────┬────────┐
│ Widget │           │ ControlBlock │ Widget │   ← один malloc
└────────┘           └──────────────┴────────┘
┌──────────────┐
│ ControlBlock │     ← второй malloc
└──────────────┘
```

Плюсы `make_shared`:

- **Быстрее** — одна аллокация вместо двух (аллокация дорогая)
- **Лучше локальность кэша** — объект и счётчики рядом

---

## Минусы `make_shared`

### 1. Память объекта не отдаётся, пока живы `weak_ptr`

Раз объект и control block в одном куске — память освобождается целиком, только когда `weak_count` тоже дойдёт до 0:

```cpp
auto s = std::make_shared<BigWidget>();   // объект + блок = один кусок
std::weak_ptr<BigWidget> w = s;
s.reset();   // strong→0: ~BigWidget() вызван, но ПАМЯТЬ (весь кусок) НЕ отдана,
             // потому что control block (weak_count=1) в том же куске
// вся память BigWidget занята, пока жив w
```

При `new` объект и блок — разные куски, память объекта отдаётся сразу при strong→0. Для больших объектов с долгоживущими `weak_ptr` это аргумент против `make_shared`.

### 2. Нельзя указать кастомный deleter

```cpp
std::shared_ptr<FILE> f(std::fopen("x","r"), &std::fclose);  // deleter — только так
auto f = std::make_shared<FILE>(...);   // невозможно передать fclose
```

`make_shared` всегда использует `delete`. Нужен кастомный deleter → используй прямой конструктор.

### 3. `private`/`protected` конструктор

`make_shared` не «друг» твоего класса, поэтому не видит закрытый конструктор:

```cpp
class Widget {
    Widget() = default;   // private
    friend ...;
public:
    static std::shared_ptr<Widget> create() {
        // return std::make_shared<Widget>();  // НЕ компилируется — нет доступа
        return std::shared_ptr<Widget>(new Widget());  // работает
    }
};
```

(Обходится трюком с локальной структурой-наследником или `std::allocate_shared`, но это уже нюанс.)

### 4. Кастомный `operator new` класса игнорируется

`make_shared` выделяет память общим аллокатором, а не через перегруженный в классе `operator new` — если он был важен, `make_shared` его обойдёт.

---

## Сводка

| |`make_unique`|`make_shared`|
|---|---|---|
|Появился|C++14|C++11|
|Аллокаций|1 (как new)|**1** (vs 2 у `shared_ptr(new)`)|
|Exception safety|да|да|
|Читаемость / нет голого new|да|да|
|Control block|нет|да, в общем куске|
|Кастомный deleter|❌ (нет и у самого unique_ptr в фабрике)|❌|
|Минус с weak_ptr|—|память объекта висит до weak→0|

---

## Практические рекомендации

- **По умолчанию** используй `make_unique` / `make_shared` вместо голого `new`.
- `make_unique` — почти всегда, минусов практически нет.
- `make_shared` — когда не нужен кастомный deleter и объект не гигантский с долгими `weak_ptr`.
- Прямой конструктор `shared_ptr(new T, deleter)` — когда нужен **кастомный deleter** (обёртки над C-API), либо когда критична раздельная аллокация ради `weak_ptr`.

Тонкость: `make_unique` **не** принимает кастомный deleter вообще — если нужен `unique_ptr` с deleter'ом (как `FilePtr` из ранее), его конструируют напрямую:

```cpp
FilePtr f(std::fopen("data.txt", "r"));   // make_unique тут не применим
```

---

## На собеседовании

Классический вопрос — **«почему `make_shared` эффективнее `shared_ptr(new T)`?»** → одна аллокация вместо двух + локальность.

Follow-up-ловушка — **«а когда `make_shared` НЕ стоит использовать?»** → кастомный deleter, приватный конструктор, большой объект с долгоживущими `weak_ptr` (память висит), перегруженный `operator new`.

И различие в мотивации: у `make_unique` выигрыш только в безопасности/читаемости (аллокация одна в любом случае), у `make_shared` — ещё и в производительности (схлопывание двух аллокаций в одну).
