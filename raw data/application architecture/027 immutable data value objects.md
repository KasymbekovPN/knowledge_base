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
// Immutable value objects: почему невозможность мутации сама по себе  
// устраняет целый класс багов - aliasing (два держателя одного объекта,  
// один меняет, оба видят изменение) и гонки данных между потоками.  
  
#include <iostream>  
#include <format>  
#include <thread>  
#include <vector>  
  
// ============================================================  
// Вариант 1: мутабельный Money - классический aliasing-баг  
// ============================================================  
namespace mutable_version {  
    namespace {  
        class Money {  
        public:  
            explicit Money(const long cents): cents_(cents) {}  
            // МУТИРУЕТ объект на месте  
            void add(const long cents) { cents_ += cents; }  
            long cents() const { return cents_; }  
        private:  
            long cents_;  
        };    
    }  
    
    static void demoAliasing() {  
        std::cout << "-- mutable version: aliasing bug --\n";  
        Money price{1'000};  
        // кто-то держит ссылку на тот же объект  
        Money& alias = price;  
        // и ещё указатель на него же  
        const Money* pointer_holder = &price;  
  
        // "локальное" изменение через alias...  
        alias.add(500);  
        std::cout << "  price.cents()           = " << price.cents()  
                  << " (changed by itself for ALL holders!)\n"  
                  << "  pointer_holder->cents()  = " << pointer_holder->cents() << " (same object)\n";  
    }  
}  
  
// ============================================================  
// Вариант 2: immutable Money - aliasing структурно невозможен  
// ============================================================  
namespace immutable_version {  
    namespace {  
        class Money {  
        public:  
            explicit Money(const long cents): cents_(cents) {}  
  
            // "Мутирующая" операция возвращает НОВОЕ значение, а не меняет текущее.  
            Money add(const long cents) { return Money{cents_ + cents}; }  
  
            long cents() const { return cents_; }  
  
            // C++20: автоматическое сравнение по значению всех полей -  
            // не нужно вручную писать operator== и рисковать забыть поле.            
            bool operator==(const Money&) const = default;  
  
        private:  
            // const - буквально невозможно изменить после конструктора  
            const long cents_;  
        };    
    }  
    
    static void demoNoAliasing() {  
        std::cout << "\n-- immutable Money: aliasing is impossible --\n";  
        Money price(1000);  
        // ссылку взять можно - но у Money нет ни одного мутирующего метода  
        Money& alias = price;  
  
        // НЕ меняет price/alias - создаёт независимое новое значение  
        const Money new_price = alias.add(500);  
        std::cout << "  price.cents()    = " << price.cents() << " (did not change)\n";  
        std::cout << "  new_price.cents() = " << new_price.cents() << " (new)\n";  
        std::cout << "  price == Money(1000): " << std::boolalpha << (price == Money(1000)) << "\n";  
        std::cout << std::noboolalpha;  
    }  
}  
  
// ============================================================  
// Потокобезопасность "бесплатно": параллельное чтение immutable Money  
// не требует синхронизации в принципе - читать нечего мутировать.  
// ============================================================  
static void concurrentReadImmutable() {  
    immutable_version::Money shared(777);  
    std::vector<std::thread> threads;  
    for (int i{}; i < 8; ++i) {  
        threads.emplace_back([&shared]() {  
            long sum{};  
            for (int j{}; j < 200'000; ++j) sum += shared.cents();  
            (void) sum;  
        });    
    }  
    
    for (auto& t: threads) t.join();  
    std::cout << "\nconcurrent read of immutable Money: finished without any mutex\n";  
}  
  
// Мутация из одного потока + чтение из другого БЕЗ синхронизации - гонка данных.  
static void concurrentMutableRace() {  
    mutable_version::Money shared(0);  
    std::thread writer{[&shared]() {  
        for (int i{}; i < 200'000; ++i) shared.add(1);  
    }};  
  
    int sum{};  
    std::thread reader{[&shared, &sum]() {  
        for (int i{}; i < 200'000; ++i) sum += shared.cents();  
    }};  
  
    writer.join();  
    reader.join();  
  
    std::cout << std::format("concurrent mutate+read of mutable Money: {}\n", sum);  
}  
  
int main() {  
    mutable_version::demoAliasing();  
    immutable_version::demoNoAliasing();  
    concurrentReadImmutable();  
    concurrentMutableRace();  
  
    return 0;  
}
```

**Value object vs entity — откуда вообще эта идея**

Value object определяется своим значением, а не идентичностью: `Money(1000)` — это просто "10 долларов", и любые два `Money(1000)` полностью взаимозаменяемы, у них нет "личности" отдельно от содержимого. Контраст — сущность вроде нашего `Order` из прошлого раза: у неё есть `id`, и она остаётся "тем же заказом", даже если поменялись количество и цена. Immutable имеет смысл именно для первого случая — там, где равенство по значению и есть весь смысл существования объекта.

**Aliasing-баг — то, что immutability убирает структурно, а не по договорённости**

`alias` и `pointerHolder` — два разных держателя **одного и того же** объекта `price`. Изменение через `alias.add(500)` изменило то, что видят все три имени сразу — `price`, `alias`, `pointerHolder` — потому что физически это один и тот же адрес в памяти. Это ровно тот же класс бага, что был в демонстрации протекающей абстракции с `getAllMutable()` несколько тем назад: любой, у кого есть ссылка/указатель на мутабельный объект, может незаметно испортить состояние для всех остальных держателей той же ссылки.

С immutable `Money` эта категория багов не "предотвращается дисциплиной программиста" — она структурно невозможна: `add()` физически не может изменить `*this`, потому что у него просто нет мутирующих методов, а `cents_` объявлено `const`. Тест это подтверждает: после `alias.add(500)` и `price`, и `alias` остались `1000`, а результат — отдельное новое значение `newPrice = 1500`.

**Потокобезопасность — не "мы были аккуратны", а факт, доказанный ThreadSanitizer'ом**

Это самый сильный практический аргумент, и он измерен, а не продекларирован. `concurrentReadImmutable()` гоняет восемь потоков, читающих один и тот же `Money` без единого мьютекса — TSan не нашёл там ни единой гонки, потому что гонка данных по определению требует хотя бы одной записи, а у immutable объекта записи нет вообще, только чтение — читать одновременно из скольких угодно потоков всегда безопасно. `concurrentMutateRace()` — тот же паттерн, но с мутабельным `Money`, где один поток пишет (`add()`), другой читает (`cents()`) без синхронизации — TSan поймал реальную гонку между конкретной строкой записи и конкретной строкой чтения, с полным stack trace обоих потоков.

Это прямая связь с самой первой темой плана — event loop и single-threaded модели: там мы обсуждали, что избежать гонок можно, заперев всю мутацию в одном потоке (strand, single-threaded reactor). Immutable value objects дают альтернативный путь к той же цели — если данные, которые нужно передать между потоками (события на шине, DTO, результаты вычислений), в принципе не могут мутироваться после создания, то ими можно свободно делиться между потоками без единого lock — не потому что мы аккуратно расставили мьютексы, а потому что мутировать нечего.

**Мы уже использовали эту идею, просто не называя её**

Ретроспективно — все наши `TaskAdded`/`OrderPlaced`/`BalanceChanged` события в `EventBus` были фактически value objects: создавались один раз при `publish()` и ни разу не мутировались после — просто мы явно не помечали поля `const` и не проговаривали это как осознанный архитектурный выбор. `operator== = default` (C++20) — приятное современное удобство: компилятор сам генерирует честное сравнение по значению всех полей, не нужно вручную писать и рисковать забыть сравнить одно из них при добавлении нового поля в будущем (ещё один пример "компилятор ловит то, что раньше приходилось проверять руками").

**Честная оговорка про цену**

"Мутация" через создание нового значения формально означает копирование состояния, а не изменение на месте — на практике move-семантика (`std::move`, move-конструкторы) делает это дешевле, чем кажется: `add()` может переиспользовать ресурсы исходного объекта при конструировании нового, а не всегда честно копировать всё с нуля. Для маленьких value types вроде `Money` (один `long`) разница вообще не измерима; для больших структур это стоит держать в уме, но обычно не является поводом отказываться от immutability там, где она реально упрощает рассуждение о коде.
