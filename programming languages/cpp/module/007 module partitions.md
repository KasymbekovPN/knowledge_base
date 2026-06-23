---
tags:
  - programming-language
  - cpp
  - module
---
[[programming languages/cpp/module/_|<=]]

# Разделы модуля (module partitions)

## Зачем они нужны

Представьте крупный модуль `graphics` — там тысячи строк: примитивы, текстуры, шейдеры, рендеринг. Складывать всё в один файл `graphics.ixx` неудобно: его тяжело читать, тяжело сопровождать, при любой правке перекомпилируется весь интерфейс.

Хочется разбить модуль на логические части — но при этом снаружи он должен оставаться **одним** модулем `graphics`. Просто завести отдельные модули `graphics_textures`, `graphics_shaders` не то же самое: это были бы разные независимые модули с разными именами, а нам нужен единый.

**Разделы** решают именно это: один модуль физически распадается на несколько файлов-частей, но логически остаётся целым.

## Синтаксис имени раздела

Раздел обозначается двоеточием после имени модуля:

```
имя_модуля : имя_раздела
```

Например, `graphics:textures` — это раздел `textures` модуля `graphics`. Двоеточие — признак раздела. Имя до двоеточия (`graphics`) — это сам модуль, имя после (`textures`) — внутреннее имя части, уникальное только в пределах этого модуля.

Важно: имя раздела видно **только внутри модуля**. Снаружи никакого `graphics:textures` не существует — потребитель импортирует просто `graphics`.

## Два вида разделов

Как и весь модуль, разделы бывают двух видов — повторяют деление «интерфейс/реализация» из Этапа 3.

### Интерфейсный раздел (interface partition)

Объявляется через `export module M:part;` — со словом `export`. Содержит часть **публичного интерфейса** модуля.

```cpp
// graphics_textures.ixx
export module graphics:textures;   // интерфейсный раздел

export class Texture {
    // ...
};

export Texture loadTexture(const char* path);
```

### Раздел реализации (implementation partition)

Объявляется через `module M:part;` — **без** `export`. Содержит внутренние детали, которые делятся между несколькими файлами модуля, но наружу не идут.

```cpp
// graphics_internal.cpp
module graphics:internal;          // раздел реализации (без export)

int allocateGpuMemory(int bytes) { // внутренний помощник
    // ...
}
```

Разница та же, что между интерфейсом и реализацией: `export module M:part` формирует видимый API, `module M:part` — приватную часть.

## Как разделы собираются в единый модуль

Ключевой момент: первичная интерфейсная единица (`export module M;` без двоеточия) должна **собрать** интерфейсные разделы воедино, импортировав и реэкспортировав их. Делается это конструкцией `export import :part;`.

Соберём пример. Модуль `graphics` из двух интерфейсных разделов плюс главный файл.

**Раздел 1 — `graphics_shapes.ixx`:**

```cpp
export module graphics:shapes;    // интерфейсный раздел

export struct Point { int x, y; };
export struct Rect { Point tl, br; };
```

**Раздел 2 — `graphics_textures.ixx`:**

```cpp
export module graphics:textures;  // интерфейсный раздел

export class Texture {
public:
    Texture(const char* path);
};
```

**Первичный интерфейс — `graphics.ixx`:**

```cpp
export module graphics;           // ПЕРВИЧНАЯ интерфейсная единица (без двоеточия)

export import :shapes;            // подключаем и реэкспортируем раздел shapes
export import :textures;          // подключаем и реэкспортируем раздел textures
```

**Потребитель — `main.cpp`:**

```cpp
import graphics;                  // импортирует ВЕСЬ модуль целиком

int main() {
    Point p{1, 2};                // из раздела shapes
    Texture t("img.png");         // из раздела textures
    // потребитель не знает и не должен знать про разделы
}
```

Обратите внимание на синтаксис `export import :shapes;` — двоеточие без имени модуля перед ним. Внутри модуля `graphics` запись `:shapes` означает «раздел `shapes` _моего_ модуля». Имя модуля повторять не нужно — оно подразумевается.

Логика конструкции:

- `import :shapes;` — подключить раздел к текущей единице
- `export import :shapes;` — подключить **и** передать его экспорт дальше, наружу модуля

Без `export` раздел был бы виден только внутри первичного файла, но не у потребителей.

## Правило: все интерфейсные разделы должны быть реэкспортированы

Это строгое требование стандарта. **Каждый интерфейсный раздел** модуля обязан быть прямо или косвенно реэкспортирован первичной интерфейсной единицей через `export import`. Нельзя завести интерфейсный раздел и «забыть» его подключить — это ошибка. Логика: интерфейсный раздел по определению содержит публичный API, поэтому он обязан стать частью единого интерфейса модуля.

На разделы реализации это правило не распространяется — они внутренние, их подключают по необходимости обычным `import :part;` (без `export`).

## Видимость между разделами

Внутри одного модуля разделы могут пользоваться друг другом. Чтобы один раздел увидел содержимое другого, он импортирует его — снова через короткий синтаксис с двоеточием:

```cpp
// graphics_textures.ixx
export module graphics:textures;

import :shapes;                   // видим раздел shapes своего модуля

export class Texture {
    Point origin;                 // используем Point из :shapes
};
```

Тонкость: когда раздел импортирует другой раздел через `import :part;` (без `export`), он **видит** его содержимое, но **не реэкспортирует** наружу. То есть `Point` доступен внутри `:textures` для работы, но через сам `:textures` потребителю не «протечёт» — наружу `Point` попадёт только потому, что первичный файл отдельно реэкспортировал `:shapes`.

## Разделы реализации и совместное использование

Разделы реализации хороши, когда несколько частей модуля используют общий внутренний код, который не должен быть виден снаружи:

**`graphics_gpu.cpp`** — общий внутренний раздел:

```cpp
module graphics:gpu;              // раздел реализации

int allocate(int bytes) { /* ... */ return 0; }
void free(int handle) { /* ... */ }
```

**`graphics_textures.cpp`** — реализация, использующая его:

```cpp
module graphics;                  // обычная единица реализации модуля

import :gpu;                      // подключаем внутренний раздел

// здесь можно вызывать allocate() / free() из :gpu
```

Так общий GPU-код живёт в одном месте, доступен разным частям модуля, но полностью скрыт от тех, кто импортирует `graphics`.

## Полная структура крупного модуля

Собрав всё вместе, типичный большой модуль выглядит так:

```
graphics.ixx              ← первичный интерфейс: export import всех :разделов
│
├── graphics_shapes.ixx   ← export module graphics:shapes   (интерфейсный раздел)
├── graphics_textures.ixx ← export module graphics:textures (интерфейсный раздел)
│
├── graphics_gpu.cpp      ← module graphics:gpu  (раздел реализации, внутренний)
└── graphics_impl.cpp     ← module graphics      (единица реализации)
```

Снаружи же всё это — один-единственный `import graphics;`.

## Разделы против отдельных модулей

Частый вопрос: когда дробить на разделы, а когда заводить отдельные модули? Ориентир такой.

Используйте **разделы**, когда части тесно связаны, образуют единый логический компонент и должны предоставляться потребителю как одно целое под одним именем. Разделы — это внутренняя организация _одного_ модуля.

Используйте **отдельные модули**, когда части самостоятельны, могут использоваться независимо и имеют смысл как отдельные единицы (например, `graphics` и `audio` — это разные модули, а не разделы одного).

Грубо говоря: разделы — про то, как _внутри_ устроен один модуль; отдельные модули — про границы между компонентами системы.

## Важные правила-памятка

Несколько ограничений, которые легко забыть:

Двоеточие в имени (`M:part`) — всегда признак раздела, и работает только **внутри** модуля `M`. Первичная интерфейсная единица — ровно одна, без двоеточия. Имена разделов уникальны в пределах модуля, но никак не видны снаружи. Все **интерфейсные** разделы обязаны быть реэкспортированы первичным файлом. Короткий синтаксис `import :part;` / `export import :part;` (без имени модуля) применяется только между единицами одного модуля.

## Сводка

|Конструкция|Что означает|
|---|---|
|`export module M:part;`|интерфейсный раздел (часть публичного API)|
|`module M:part;`|раздел реализации (внутренний)|
|`export import :part;`|реэкспорт интерфейсного раздела наружу (в первичном файле)|
|`import :part;`|подключить раздел для внутреннего использования|
|`import M;` (у потребителя)|импорт всего модуля целиком, без знания о разделах|

## Что вы освоили

Вы умеете разбивать большой модуль на интерфейсные разделы и разделы реализации, собирать их в единый публичный интерфейс через `export import :part;`, организовывать общий внутренний код в разделах реализации и понимаете правило обязательного реэкспорта интерфейсных разделов. Главное — видите разницу между «разбить один модуль на части» (разделы) и «разделить систему на компоненты» (отдельные модули).

### graphics_gpu.cppm
```cpp
module;

#include <iostream>
#include <format>

export module graphics:gpu;

int allocate_it(int bytes) {
    std::cout << std::format("allocated {}\n", bytes);
    return 0;
}

void free_it(int handle) {
    std::cout << std::format("free {}\n", handle);
}
```

### graphics_shapes.cppm
```cpp
module;

#include <iostream>
#include <format>

export module graphics:shapes;

export struct Point {
    double x, y;

    void print() const {
        std::cout << std::format("Point [{}, {}]\n", x, y);
    }
};

export struct Rect {
    Point tl, br;

    void print() const {
        std::cout << "Rect\n";
        tl.print();
        br.print();
    }
};
```

### graphics_textures.cppm
```cpp
export module graphics:textures;

import :shapes;
import :gpu;

export class Texture {
public:
    Texture(Point _point, const char* _path):
        point{_point},
        path{_path} {}
    void print() const;
private:
    const char* path;
    Point point;
};
```

### graphics_textures_impl.cpp
```cpp
module;

#include <iostream>
#include <format>

module graphics;

void Texture::print() const {
    allocate_it(42);
    free_it(43);
    std::cout << std::format("path: {}\n", path);
}
```

### demo.cpp
```cpp
/*

clang++ -std=c++23 --precompile graphics_shapes.cppm -o graphics_shapes.pcm;

if ($?) { clang++ -std=c++23 --precompile graphics_gpu.cppm -o graphics_gpu.pcm }

if ($?) { clang++ -std=c++23 --precompile "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" graphics_textures.cppm -o graphics_textures.pcm }

if ($?) { clang++ -std=c++23 --precompile "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:textures=graphics_textures.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" graphics.cppm -o graphics.pcm }

if ($?) { clang++ -std=c++23 "-fmodule-file=graphics=graphics.pcm" "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:textures=graphics_textures.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" demo.cpp graphics_textures_impl.cpp graphics.pcm graphics_shapes.pcm graphics_textures.pcm graphics_gpu.pcm -o app.exe }

*/

import graphics;

#include <iostream>

void test_point() {
    std::cout << "### test_point\n";
    Point p{42, 12.34};
    p.print();
}

void test_rect() {
    std::cout << "### test_point\n";
    Rect rect{{142, 112.34}, {242, 212.34}};
    rect.print();
}

void test_texture() {
    std::cout << "### test_texture\n";
    Texture tex{{1, 2}, "some-path"};
    tex.print();
}

int main() {
    test_point();
    test_rect();
    test_texture();

    return 0;
}
```

