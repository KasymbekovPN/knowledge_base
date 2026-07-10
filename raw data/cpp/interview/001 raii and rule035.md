
[[raw data/cpp/interview/_|<=]]

# RAII и правило 0/3/5

## RAII (Resource Acquisition Is Initialization)

Идея: **ресурс привязан к времени жизни объекта**. Захват в конструкторе, освобождение в деструкторе. Раз деструкторы в C++ вызываются детерминированно (в т.ч. при раскрутке стека из-за исключения), ресурс освободится всегда.

```cpp
class File {
    FILE* f_;
public:
    explicit File(const char* path) : f_(std::fopen(path, "r")) {
        if (!f_) throw std::runtime_error("open failed");   // захват
    }
    ~File() { if (f_) std::fclose(f_); }                    // освобождение

    FILE* get() const { return f_; }
};

void process() {
    File f("data.txt");   // открыли
    mayThrow();           // даже если бросит — ~File() закроет файл
}                         // закрыли автоматически
```

Ресурс — не только память: файлы, мьютексы (`lock_guard`), сокеты, хендлы, транзакции. `unique_ptr`, `lock_guard`, `vector`, `string` — всё это RAII-обёртки.

Ключевое преимущество: **exception safety «бесплатно»**. Не нужны `try/finally` (которого в C++ и нет) — очистка гарантирована самим языком.

`f_` не указывает на память, выделенную через `new`. `std::fopen` — это функция C, которая возвращает указатель на `FILE`, но **не** через `new`. Внутри она использует свой механизм аллокации (обычно `malloc` + системный вызов открытия файла на уровне ОС). Правило простое: **освобождать нужно тем же механизмом, которым выделяли.**

```
fopen  → fclose      (парные C-функции)
malloc → free
new    → delete
new[]  → delete[]
```

Смешивать их — UB:

```cpp
FILE* f = std::fopen("x", "r");
delete f;    // UB! fopen выделял не через new
free(f);     // тоже UB! это не «сырой» malloc-блок, а внутренняя структура FILE
```

## Что делает `std::fclose`

`fclose` делает **две вещи**, которые `delete`/`free` сделать не могут:

1. Сбрасывает буферы и **закрывает файловый дескриптор** на уровне ОС (освобождает системный ресурс — хендл/fd).
2. Освобождает внутреннюю структуру `FILE`, которую выделил `fopen`.

То есть `fclose` — это и есть полное освобождение ресурса. Больше ничего вызывать не нужно.

## Общий принцип для RAII-обёрток

Деструктор должен вызывать функцию, **парную** той, что захватила ресурс:

```cpp
class Socket {
    int fd_;
public:
    Socket(...) : fd_(::socket(...)) { /*...*/ }
    ~Socket() { if (fd_ >= 0) ::close(fd_); }   // close, парный socket() — не delete
};

class Lock {
    std::mutex& m_;
public:
    Lock(std::mutex& m) : m_(m) { m_.lock(); }
    ~Lock() { m_.unlock(); }                     // unlock, парный lock()
};
```

`delete` в деструкторе появляется **только** если в конструкторе (или где-то в классе) был `new`:

```cpp
class Widget {
    Impl* impl_;
public:
    Widget() : impl_(new Impl()) {}   // выделили через new
    ~Widget() { delete impl_; }       // → парный delete
};
```

## На собеседовании

Это хороший маркер понимания: интервьюер может спросить «а почему в деструкторе `fclose`, а не `delete`?». Правильный ответ — про **симметрию захвата и освобождения** и про то, что `FILE*` — это handle к ресурсу ОС, а не указатель на C++-объект в куче. Один и тот же указательный тип `FILE*` не означает, что это `new`-память.

Кстати, идеально «раульный» вариант этого класса вообще не пишет деструктор руками, а оборачивает `FILE*` в `unique_ptr` с кастомным deleter (Rule of Zero):

```cpp
struct FileCloser {
    void operator()(FILE* f) const noexcept { if (f) std::fclose(f); }
};
using FilePtr = std::unique_ptr<FILE, FileCloser>;

FilePtr f(std::fopen("data.txt", "r"));   // fclose вызовется автоматически
```

Такой ответ на собеседовании показывает, что ты знаешь и «ручной» RAII, и идиоматичный современный подход.


# `std::unique_ptr` с кастомным deleter

## Сигнатура шаблона

```cpp
template<class T, class Deleter = std::default_delete<T>>
class unique_ptr;
```

Второй параметр — **тип deleter'а**. По умолчанию это `std::default_delete<T>`, который просто вызывает `delete`. Подставив свой тип, ты меняешь то, что происходит при разрушении.

```cpp
std::unique_ptr<int>                       // deleter = default_delete<int> → delete
std::unique_ptr<FILE, FileCloser>          // deleter = FileCloser → fclose
```

## Разбор конструкции

```cpp
struct FileCloser {
    void operator()(FILE* f) const noexcept { if (f) std::fclose(f); }
};
using FilePtr = std::unique_ptr<FILE, FileCloser>;

FilePtr f(std::fopen("data.txt", "r"));
```

- `FileCloser` — функтор (класс с `operator()`), принимающий указатель и освобождающий ресурс.
- `unique_ptr` при разрушении вызовет `deleter(ptr)` вместо `delete ptr`.
- `noexcept` важен — deleter'ы не должны бросать исключения (их зовут из деструктора).

## Три способа задать deleter

### 1. Функтор (рекомендуется)

```cpp
struct FileCloser {
    void operator()(FILE* f) const noexcept { if (f) std::fclose(f); }
};
using FilePtr = std::unique_ptr<FILE, FileCloser>;

FilePtr f(std::fopen("x", "r"));
```

**Размер: `sizeof(FilePtr) == sizeof(FILE*)`** — один указатель. Функтор без состояния (пустой класс) не занимает места в `unique_ptr` благодаря **empty base optimization**. Это лучший вариант.

### 2. Указатель на функцию

```cpp
using FilePtr = std::unique_ptr<FILE, decltype(&std::fclose)>;

FilePtr f(std::fopen("x", "r"), &std::fclose);   // deleter надо передать в конструктор
```

**Размер: `sizeof(FilePtr) == 2 * sizeof(void*)`** — объект хранит ещё и указатель на функцию. Работает, но толще и требует передавать deleter в конструктор.

### 3. Лямбда

```cpp
auto closer = [](FILE* f) noexcept { if (f) std::fclose(f); };
using FilePtr = std::unique_ptr<FILE, decltype(closer)>;

FilePtr f(std::fopen("x", "r"), closer);
```

С C++20 лямбды без захвата default-конструируемы, до C++20 её нужно передавать в конструктор. По размеру как функтор (пустая лямбда без захвата), но синтаксис с `decltype` менее удобен для переиспользования.

## Почему deleter — часть типа (важный момент)

У `unique_ptr` тип deleter'а — **параметр шаблона**, то есть часть типа. Это значит:

```cpp
std::unique_ptr<FILE, FileCloser> a;
std::unique_ptr<FILE>             b;   // РАЗНЫЕ типы, не совместимы
```

Плюс: **нулевой оверхед** для stateless-deleter'а (empty base optimization → размер = один указатель).

Контраст с `shared_ptr`, где deleter хранится в control block (type erasure) и **не** входит в тип:

```cpp
std::shared_ptr<FILE> sp(std::fopen("x","r"), &std::fclose);  // тип просто shared_ptr<FILE>
```

Это классический вопрос на собеседовании: _почему у `unique_ptr` deleter в типе, а у `shared_ptr` — нет?_ Ответ: `unique_ptr` спроектирован под zero-overhead (deleter в типе, EBO), а `shared_ptr` и так платит за control block, поэтому прячет deleter туда ради гибкости (можно менять deleter в рантайме, класть в контейнер разные).

## Практическое применение — обёртки над C-API

Идиома для любого ресурса с парой create/destroy:

```cpp
// POSIX socket через unique_ptr — но fd это int, не указатель...
// для не-указательных handle нужен pointer typedef в deleter:
struct FdDeleter {
    using pointer = int;                    // сообщаем unique_ptr, что "указатель" это int
    void operator()(int fd) const noexcept { if (fd >= 0) ::close(fd); }
};
using FdPtr = std::unique_ptr<int, FdDeleter>;
```

Более частый случай — библиотеки вроде OpenSSL, SQLite, libcurl:

```cpp
struct SqliteDeleter {
    void operator()(sqlite3* db) const noexcept { sqlite3_close(db); }
};
using SqlitePtr = std::unique_ptr<sqlite3, SqliteDeleter>;
```

## Что это даёт

```cpp
void process() {
    FilePtr f(std::fopen("data.txt", "r"));
    if (!f) throw std::runtime_error("open failed");

    mayThrow();                    // исключение? fclose всё равно вызовется
    std::fgets(buf, n, f.get());   // .get() — сырой FILE* для C-API
}                                  // fclose автоматически, ноль ручного кода
```

Получаешь Rule of Zero: не пишешь деструктор, copy запрещён автоматически (`unique_ptr` некопируем), move работает из коробки, exception safety гарантирована.

Хороший ход на собеседовании — упомянуть, что для non-nullable семантики (`fd == -1` как «пусто») в deleter добавляют вложенный тип `pointer`, и `unique_ptr` начинает работать не с `T*`, а с этим типом. Это показывает глубокое понимание того, что `unique_ptr` управляет не обязательно «настоящим указателем», а любым handle, удовлетворяющим NullablePointer.

## Правило пяти специальных функций

Компилятор может сгенерировать пять специальных методов:

```cpp
class T {
    ~T();                          // 1. деструктор
    T(const T&);                   // 2. copy constructor
    T& operator=(const T&);        // 3. copy assignment
    T(T&&) noexcept;               // 4. move constructor
    T& operator=(T&&) noexcept;    // 5. move assignment
};
```

## Правило трёх (C++98)

Если тебе нужен **хотя бы один** из {деструктор, copy-конструктор, copy-присваивание} — почти наверняка нужны **все три**. Признак: класс владеет ресурсом, требующим ручного управления.

Классическая ловушка — определить только деструктор:

```cpp
class BadString {
    char* data_;
public:
    BadString(const char* s) { data_ = strdup(s); }
    ~BadString() { free(data_); }
    // copy-конструктор сгенерирован по умолчанию → поверхностное копирование!
};

BadString a("hi");
BadString b = a;   // b.data_ == a.data_ (тот же указатель!)
// при разрушении обоих → double-free (UB)
```

Нужно определить и copy-конструктор, и copy-присваивание (с корректной семантикой копирования + защитой от самоприсваивания).

## Правило пяти (C++11)

С добавлением move-семантики: если управляешь ресурсом, определяй все **пять**. Move-операции позволяют «украсть» ресурс вместо копирования:

```cpp
class String {
    char* data_ = nullptr;
    size_t size_ = 0;
public:
    String(const char* s) : size_(std::strlen(s)) {
        data_ = new char[size_ + 1];
        std::memcpy(data_, s, size_ + 1);
    }

    ~String() { delete[] data_; }

    // copy — глубокое копирование
    String(const String& o) : size_(o.size_) {
        data_ = new char[size_ + 1];
        std::memcpy(data_, o.data_, size_ + 1);
    }
    String& operator=(const String& o) {
        if (this != &o) {                    // защита от самоприсваивания
            char* tmp = new char[o.size_ + 1];   // сначала выделяем (strong guarantee)
            std::memcpy(tmp, o.data_, o.size_ + 1);
            delete[] data_;
            data_ = tmp;
            size_ = o.size_;
        }
        return *this;
    }

    // move — забираем ресурс, обнуляем источник
    String(String&& o) noexcept : data_(o.data_), size_(o.size_) {
        o.data_ = nullptr;
        o.size_ = 0;
    }
    String& operator=(String&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            data_ = o.data_;
            size_ = o.size_;
            o.data_ = nullptr;
            o.size_ = 0;
        }
        return *this;
    }
};
```

Почему move должен быть `noexcept`: контейнеры (например `vector` при реаллокации) используют move только если он `noexcept`, иначе откатываются на copy ради strong exception guarantee.

## Правило нуля (Rule of Zero) — предпочтительное

**Не пиши ни одной из пяти функций.** Вместо ручного управления используй типы, которые уже RAII (`unique_ptr`, `shared_ptr`, `vector`, `string`). Тогда компилятор сгенерирует все пять корректно.

```cpp
class Widget {
    std::unique_ptr<Impl> impl_;      // владение через smart pointer
    std::vector<int> data_;           // владеет своим буфером сам
    std::string name_;
    // ничего писать не нужно — copy/move/destroy корректны автоматически
};
```

Это идиоматичный современный C++: ручное управление ресурсом инкапсулируется в один маленький RAII-класс (или готовый тип), а «бизнес-классы» следуют Rule of Zero.

## Важные нюансы для собеседования

**Объявление деструктора подавляет генерацию move-операций.** Если написал `~T()`, компилятор **не** сгенерирует move-конструктор и move-присваивание → класс будет копироваться там, где мог бы перемещаться. Отсюда практическое усиление правила: объявил одно из пяти — осознанно реши про остальные.

**`= default` и `= delete`** — можно явно запросить или запретить операцию:

```cpp
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;             // копирование запрещено
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;                 // move разрешён
    NonCopyable& operator=(NonCopyable&&) = default;
};
```

**Copy-and-swap идиома** — элегантная реализация присваивания, дающая strong exception guarantee и переиспользующая copy-конструктор:

```cpp
String& operator=(String o) {   // параметр по значению — копия уже сделана
    swap(*this, o);             // обмениваемся содержимым
    return *this;               // старое содержимое умрёт вместе с o
}                               // работает и для copy, и для move присваивания
```

Отличие от Java: там нет деструкторов и RAII, очистка ресурсов — через `try-with-resources`/`AutoCloseable` или `finally`, а память — на GC. В C++ детерминированное разрушение делает RAII фундаментом управления **любыми** ресурсами. Именно поэтому «правило 0/3/5» — почти обязательный вопрос: он проверяет, понимаешь ли ты семантику владения и копирования/перемещения.
