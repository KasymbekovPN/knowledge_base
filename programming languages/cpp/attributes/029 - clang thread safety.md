---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

Прежде чем объяснять — важная оговорка: это анализ, специфичный **только для Clang** (`-Wthread-safety`), GCC его вообще не реализует.

Полный набор атрибутов Clang Thread Safety Analysis (`-Wthread-safety`), от объявления типа-капабилити до отключения анализа:

|Атрибут (`__attribute__((...))`)|К чему применяется|Что означает|
|---|---|---|
|`capability("type")`|класс|Помечает тип как "капабилити" (объект блокировки) — только такие типы можно использовать в `guarded_by`, `requires_capability` и т.п. Строка `"type"` — просто описание для сообщений об ошибках (`"mutex"`, `"role"`).|
|`scoped_lockable`|класс|Помечает RAII-обёртку (аналог `std::lock_guard`) — захват происходит в конструкторе, освобождение в деструкторе, без отдельных явных вызовов Lock/Unlock.|
|`guarded_by(c)`|поле данных|Поле можно читать/писать только когда захвачен мьютекс `c`. Обращение без захвата — ошибка компиляции.|
|`pt_guarded_by(c)`|поле-указатель/умный указатель|То же самое, но защищает не сам указатель, а данные, на которые он указывает (`int* p PT_GUARDED_BY(mu)` — `p` можно читать всегда, `*p` только под `mu`).|
|`acquired_before(...)`|поле-мьютекс|Задаёт порядок захвата: этот мьютекс всегда должен захватываться раньше перечисленных — помогает статически ловить потенциальный deadlock из-за разного порядка блокировок в разных местах кода.|
|`acquired_after(...)`|поле-мьютекс|Симметрично — этот мьютекс должен захватываться позже перечисленных.|
|`requires_capability(...)` / `requires_shared_capability(...)`|функция/метод|Вызывающий обязан уже держать перечисленные капабилити (эксклюзивно / в разделяемом режиме) до вызова этой функции. Сама функция ничего не захватывает и не освобождает.|
|`acquire_capability(...)` / `acquire_shared_capability(...)`|функция/метод|Функция захватывает перечисленные капабилити и не освобождает их до выхода (типично для `Lock()`/`ReaderLock()`).|
|`release_capability(...)` / `release_shared_capability(...)` / `release_generic_capability(...)`|функция/метод|Функция освобождает перечисленные капабилити (типично для `Unlock()`); `_generic_` — когда неизвестно заранее, в каком режиме (эксклюзивном/shared) был захват.|
|`try_acquire_capability(...)` / `try_acquire_shared_capability(...)`|функция/метод, возвращающая `bool`|Захватывает капабилити только если функция вернула `true` (аналог `try_lock()`) — анализ учитывает это условие в разных ветках `if`.|
|`locks_excluded(...)`|функция/метод|Функцию нельзя вызывать, если перечисленные капабилити уже захвачены — защита от deadlock при повторном захвате нереентерабельного мьютекса или от инверсии порядка блокировок.|
|`assert_capability(...)` / `assert_shared_capability(...)`|функция/метод|Функция runtime-проверяет (например, через `assert`), что капабилити захвачена, и сообщает об этом статическому анализатору — способ "внедрить" знание в места, которые сам анализ не может отследить синтаксически.|
|`no_thread_safety_analysis`|функция/метод|Полностью отключает проверку для этой функции — люк для кода с паттернами блокировки, которые анализатор не может корректно понять.|

Дополнительно стоит знать про **устаревшие имена** тех же концепций — старые кодовые базы (и старые версии Abseil) могут использовать до-`capability`-модельные атрибуты: `lockable` (= `capability`), `exclusive_lock_function`/`shared_lock_function` (= `acquire_capability`/`acquire_shared_capability`), `unlock_function` (= `release_capability`), `exclusive_trylock_function`/`shared_trylock_function` (= `try_acquire_capability`/`try_acquire_shared_capability`), `exclusive_locks_required`/`shared_locks_required` (= `requires_capability`/`requires_shared_capability`). Функционально они эквивалентны, но современная документация LLVM рекомендует именно `capability`-варианты из таблицы выше.

**Идея.** Это не рантайм-проверка (как `std::mutex`'s `try_lock` или ассерты), а статический анализ на этапе компиляции: Clang отслеживает, какие мьютексы "держит" каждый путь выполнения, и проверяет, что доступ к помеченным данным происходит только под правильным мьютексом — примерно как type-checking, только для блокировок.

```cpp
#include <mutex>

// Макросы-обёртки — обычно именно так это и используют на практике
// (см. abseil mutex.h), потому что голый __attribute__ громоздкий
#define GUARDED_BY(x)          __attribute__((guarded_by(x)))
#define REQUIRES(...)          __attribute__((requires_capability(__VA_ARGS__)))
#define ACQUIRE(...)           __attribute__((acquire_capability(__VA_ARGS__)))
#define RELEASE(...)           __attribute__((release_capability(__VA_ARGS__)))
#define CAPABILITY(x)          __attribute__((capability(x)))
#define SCOPED_CAPABILITY      __attribute__((scoped_lockable))
#define NO_THREAD_SAFETY_ANALYSIS __attribute__((no_thread_safety_analysis))

class BankAccount {
public:
    void deposit(int amount) {
        std::lock_guard<std::mutex> lock(mu_);
        balance_ += amount;              // ok — mu_ захвачен через lock_guard
    }

    int getBalanceUnsafe() const {
        return balance_;                 // ОШИБКА КОМПИЛЯЦИИ: обращение к
                                          // balance_ без захвата mu_
    }

private:
    std::mutex mu_;
    int balance_ GUARDED_BY(mu_) = 0;    // balance_ можно трогать только под mu_
};
```

Чтобы это заработало со стандартным `std::mutex`, Clang нужно объяснить, что `std::lock_guard`/`std::mutex` вообще являются "капабилити" (объектами блокировки) — стандартная библиотека сама этими атрибутами не помечена, поэтому на практике почти всегда используют готовую обёртку (как `absl::Mutex` из Abseil) с уже расставленными аннотациями, а не голый `std::mutex`:

```cpp
class CAPABILITY("mutex") Mutex {
public:
    void Lock() ACQUIRE() {}
    void Unlock() RELEASE() {}
};

class SCOPED_CAPABILITY MutexLock {
public:
    explicit MutexLock(Mutex* mu) ACQUIRE(mu) : mu_(mu) { mu_->Lock(); }
    ~MutexLock() RELEASE() { mu_->Unlock(); }
private:
    Mutex* mu_;
};

class BankAccount {
public:
    void deposit(int amount) {
        MutexLock lock(&mu_);
        balance_ += amount;      // ok
    }

    int getBalanceUnsafe() {
        return balance_;         // ошибка: чтение guarded_by-поля без лока
    }

    void requireLocked() REQUIRES(mu_) {
        balance_ += 1;           // ok — вызывающий обязан уже держать mu_
    }

private:
    Mutex mu_;
    int balance_ GUARDED_BY(mu_) = 0;
};
```

Собирается с обязательным флагом:

```bash
clang++ -Wthread-safety -std=c++17 -c file.cpp
```

Что реально ловит на этапе компиляции:

- Обращение к `GUARDED_BY`-полю без захваченного мьютекса.
- Вызов функции, помеченной `REQUIRES(mu)`, когда `mu` точно не захвачен в этой точке кода (анализ отслеживает состояние блокировок по путям выполнения, включая условные ветки).
- Захват мьютекса и невозврат (забытый `Unlock`) на некоторых путях выхода из функции.
- Двойной захват незашедуемого (`non-reentrant`) мьютекса тем же потоком в рамках прослеживаемого статически пути.

Важные ограничения:

- Это **не** доказательство отсутствия data race в общем смысле — анализ основан на аннотациях и синтаксическом отслеживании локов, а не на полноценной верификации; данные, доступные не через помеченное поле (например, через сырой указатель, `const_cast`, или переданные в другой поток без аннотаций), не проверяются.
- Требует последовательной разметки всей цепочки — если где-то забыть аннотацию (например, на функции, которая внутри лочит мьютекс, но сама не помечена `REQUIRES`), анализ на этом участке "слепнет".
- `NO_THREAD_SAFETY_ANALYSIS` — люк для кода, который анализатор не может корректно понять (нестандартные паттерны блокировки), стоит использовать точечно и с комментарием почему.
- Стандартного `[[...]]`-аналога нет; в стандартный C++ эта концепция не вошла (хотя обсуждалась), поэтому остаётся Clang-специфичным расширением, которое активно используется в крупных кодовых базах (Chromium, Abseil/Google-код), но не переносимо на GCC/MSVC вообще — обычно оборачивают в макросы, которые на других компиляторах разворачиваются в пустоту.

### Пример

```cpp
#include <iostream>  
#include <mutex>  
  
// Макросы-обёртки — обычно именно так это и используют на практике  
// (см. abseil mutex.h), потому что голый __attribute__ громоздкий  
#define GUARDED_BY(x) __attribute__((guarded_by(x)))  
#define REQUIRES(...) __attribute__((requires_capability(__VA_ARGS__)))  
#define ACQUIRE(...) __attribute__((acquire_capability(__VA_ARGS__)))  
#define RELEASE(...) __attribute__((release_capability(__VA_ARGS__)))  
#define CAPABILITY(x) __attribute__((capability(x)))  
#define SCOPED_CAPABILITY __attribute__((scoped_lockable))  
#define NO_THREAD_SAFETY_ANALYSIS __attribute__((no_thread_safety_analysis))  
  
namespace demo0 {  
    class BankAccount {  
    public:  
        void deposit(const int amount) {  
            std::lock_guard<std::mutex> lock{mu_};  
            // ok — mu_ захвачен через lock_guard  
            balance_ += amount;  
        }  
        // ОШИБКА КОМПИЛЯЦИИ: обращение к balance_ без захвата mu_  
        int getBalanceUnsafe() const {  
            return balance_;  
        }  
    private:  
        std::mutex mu_;  
        // balance_ можно трогать только под mu_  
        int balance_ GUARDED_BY(mu_) = 0;  
    };}  
  
namespace demo1 {  
  
    class CAPABILITY("mutex") Mutex {  
    public:  
        void Lock() ACQUIRE() {}  
        void Unlock() RELEASE() {}  
    };  

    class SCOPED_CAPABILITY MutexLock {  
    public:  
        explicit MutexLock(Mutex* mu) ACQUIRE(mu): mu_{mu} {  
            mu_->Lock();  
        }        
        ~MutexLock() RELEASE() { mu_->Unlock(); }  
  
    private:  
        Mutex* mu_{nullptr};  
    };  

    class BankAccount {  
    public:  
        // ok  
        void deposit(const int amount) {  
            MutexLock lock{&mu_};  
            balance_ += amount;  
        }  
        // ошибка: чтение guarded_by-поля без лока  
        int getBalanceUnsafe() const {  
            return balance_;  
        }  
        // ok — вызывающий обязан уже держать mu_  
        void requireLocked() REQUIRES(mu_) {  
            balance_ += 1;  
        }  
    private:  
        Mutex mu_;  
        int balance_ GUARDED_BY(mu_) = 0;  
    };  
}  
  
int main() {  
    auto ba0 = demo0::BankAccount();  
    ba0.deposit(10);  
    std::cout << ba0.getBalanceUnsafe() << std::endl;  
  
    auto ba1 = demo1::BankAccount();  
    ba1.deposit(10);  
    std::cout << ba1.getBalanceUnsafe() << std::endl;  
  
    return 0;  
}  
  
/*  
  
clang++.exe -Wthread-safety -std=c++23 -c main.cpp  
  
*/
```
