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
    ],    "buildPresets": [  
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
cmake_minimum_required(VERSION 3.30)  
project(app CXX)  
  
add_executable(app  
        app/main.cpp        
        src/domain.cpp        
        src/infra_stripe.cpp)  
target_compile_features(app PUBLIC cxx_std_23)  
target_include_directories(app PRIVATE include)  
  
enable_testing()  
  
add_executable(domain_test  
        tests/domain_test.cpp        
        src/domain.cpp)  
target_compile_features(domain_test PUBLIC cxx_std_23)  
target_include_directories(domain_test PRIVATE include)  
  
add_test(NAME domain_test COMMAND domain_test)
```

### app/main.cpp
```cpp
// Composition root: единственное место во всей программе, где domain  
// и infra встречаются. Здесь оба #include легальны и ожидаемы.  
  
#include "domain.hpp"  
#include "infra_stripe.hpp"  
  
int main() {  
    // low-level деталь  
    StripeGateway gateway;  
    // high-level политика, DI через конструктор  
    OrderProcessor processor{gateway};  
    processor.placeOrder("laptop", 999.99);  
  
    return 0;  
}
```

### tests/domain_test.cpp
```cpp
// Тест ТОЛЬКО высокоуровневого модуля. Обратите внимание: этот файл вообще  
// не инклудит infra_stripe.hpp и не знает о Stripe. Это возможно ровно  
// потому, что domain зависит от собственной абстракции IPaymentGateway,  
// а не от какой-то конкретной реализации - т.е. прямое следствие DIP.  
  
#include "domain.hpp"  
  
#include <cassert>  
#include <iostream>  
  
class FakeGateway : public IPaymentGateway {  
public:  
    bool charge(const double amount) override {  
        lastAmount = amount;  
        return shouldSucceed;  
    }  
    double lastAmount{};  
    bool shouldSucceed{true};  
};  
  
int main() {  
    FakeGateway fake;  
    OrderProcessor processor{fake};  
  
    bool result{processor.placeOrder("widget", 42.0)};  
    assert(result == true);  
    assert(fake.lastAmount == 42.0);  
    std::cout << "[OK] domain_test: success order\n";  
  
    fake.shouldSucceed = false;  
    result = processor.placeOrder("gadget", 10.0);  
    assert(result == false);  
    std::cout << "[OK] domain_test: cancelled payment\n";  
  
    return 0;  
}
```

### include/domain.hpp
```cpp
// domain.hpp — ВЫСОКОУРОВНЕВЫЙ модуль (бизнес-правила: "как оформляется заказ").  
// Ключевой момент DIP: интерфейс IPaymentGateway объявлен ЗДЕСЬ, в модуле,  
// которому он нужен, а не в infra-модуле, который его будет реализовывать.  
// Этот заголовок НЕ включает ничего из infra_stripe - domain вообще не  
// подозревает о существовании Stripe, PayPal или кого угодно ещё.  
  
#pragma once  
  
#include <string>  
  
// Абстракция, которую определяет высокоуровневый модуль под свои нужды.  
class IPaymentGateway {  
public:  
    virtual ~IPaymentGateway() = default;  
    virtual bool charge(const double amount) = 0;  
};  
  
// Высокоуровневая политика: "как оформляется заказ" не зависит от того,  
// КАКОЙ платёжный шлюз стоит за IPaymentGateway.  
class OrderProcessor {  
public:  
    explicit OrderProcessor(IPaymentGateway& gateway);  
    bool placeOrder(const std::string& item, const double amount) const;  
private:  
    IPaymentGateway& gateway_;  
};
```

### src/domain.cpp
```cpp
#include "domain.hpp"  
  
#include <iostream>  
#include <format>  
  
OrderProcessor::OrderProcessor(IPaymentGateway& gateway):  
    gateway_(gateway) {}  
  
bool OrderProcessor::placeOrder(const std::string &item, const double amount) const {  
    std::cout << std::format("[domain] placing the order {} for ${}\n", item, amount);  
    const bool ok{gateway_.charge(amount)};  
    if (ok) {  
        std::cout << "order confirmed\n";  
    } else {  
        std::cout << "payment failed, order cancelled\n";  
    }  
    return ok;  
}
```

### include/infra_stripe.hpp
```cpp
// infra_stripe.hpp — НИЗКОУРОВНЕВЫЙ модуль (конкретная деталь: конкретный  
// платёжный провайдер). Обратите внимание на направление #include:  
// infra зависит от domain (реализует его интерфейс), а не наоборот.  
// Именно это и называется "инверсией" в Dependency Inversion - стрелка  
// зависимости идёт от низкоуровневой детали к высокоуровневой абстракции,  
// а не как в "наивной" многослойной архитектуре, где верхний слой обычно  
// зависит от нижнего.  
#pragma once  
  
#include "domain.hpp"  
  
class StripeGateway: public IPaymentGateway {  
public:  
    bool charge(const double amount) override;  
};
```

### src/src/infra_stripe.cpp
```cpp
#include "infra_stripe.hpp"  
  
#include <iostream>  
#include <format>  
  
bool StripeGateway::charge(const double amount) {  
    std::cout << std::format("StripeGateway::charge ${}\n", amount);  
    return true;  
}
```

Single Responsibility — у класса должна быть одна причина для изменения. В C++ это особенно легко нарушить незаметно: класс, который одновременно парсит данные, валидирует их и сериализует — при первом взгляде выглядит невинно, но у него уже три независимых причины меняться. Практический признак нарушения в C++ — класс с методами, которые логически группируются в непересекающиеся кластеры и почти не трогают одни и те же приватные поля.

Open/Closed — открыт для расширения, закрыт для модификации. В C++ это достигается двумя принципиально разными способами: рантайм-полиморфизмом (виртуальные функции, как в примере ниже) или compile-time полиморфизмом через шаблоны/CRTP — вторая версия не платит за vtable и не требует наследования вообще, но платит временем компиляции и, если не аккуратно, раздутием кода (template bloat).

Liskov Substitution — там, где C++ подкидывает специфичные ловушки. Object slicing — если функция принимает `Base` по значению, а не по ссылке/указателю, любая специфика `Derived` физически обрезается при копировании; формально это не нарушение LSP как такового, но частый источник багов, маскирующихся под него. Невиртуальный деструктор в базовом классе — `delete` через `Base*`, указывающий на `Derived`, не вызовет деструктор `Derived` — undefined behavior, а не просто "не самый чистый код". И тонкость самого принципа: переопределение не должно ужесточать предусловия или ослаблять постусловия — если `Derived::charge()` внезапно требует `amount > 0` (кидает исключение на 0), хотя `Base::charge()` спокойно принимал 0 — код, полагающийся на контракт базового класса, ломается именно там, где ожидал полиморфной подстановки.

Interface Segregation — много узких интерфейсов лучше одного толстого. В C++ это особенно дёшево реализовать, потому что множественное наследование от чисто абстрактных классов (без данных, только чистые virtual) не создаёт проблем ромбовидного наследования — там нечему конфликтовать, если нет общих полей. Практически: вместо одного `IDevice` с двадцатью методами — `IReadable`, `IWritable`, `IConfigurable` по отдельности, класс наследует только то, что реально реализует.

**Dependency Inversion — почему это не то же самое, что просто "используй интерфейсы"**

Формулировка принципа состоит из двух частей, и вторая часть обычно теряется в упрощённых пересказах: (1) высокоуровневые модули не должны зависеть от низкоуровневых — оба должны зависеть от абстракций, и (2) абстракции не должны зависеть от деталей — детали должны зависеть от абстракций. Ключевое слово в названии — "инверсия": речь не про "добавь интерфейс между слоями", а про то, что стрелка зависимости между модулями физически разворачивается относительно наивной многослойной архитектуры.

В примере это видно буквально на уровне `#include` и объектных файлов. `domain.hpp` — высокоуровневый модуль ("как оформляется заказ") — объявляет `IPaymentGateway` сам, под свои нужды, и ничего не знает про `infra_stripe.hpp`. `infra_stripe.hpp` — низкоуровневая деталь (конкретный платёжный провайдер) — инклудит `domain.hpp`, чтобы реализовать его интерфейс. Если бы это была "наивная" многослойная архитектура без инверсии, было бы наоборот: интерфейс платежей жил бы в infra-слое, а domain бы его инклудил — и тогда domain был бы физически привязан к infra на уровне заголовков, даже если бы вызывал методы только через абстрактный класс.

Разница не абстрактная, а измеримая: тест подтвердил, что `domain.o` компилируется и линкуется в `domain_test` без единого упоминания `infra_stripe.hpp`, и `nm domain.o | grep -i stripe` не находит вообще ничего — символов Stripe там физически нет, потому что их там никогда и не было. Это значит: бизнес-логику можно тестировать, не подключая Stripe SDK, не поднимая сеть, не собирая тяжёлую внешнюю зависимость — и именно об этом следующая тема плана (модульность): DIP — это не про "красивый код", а про то, что реально можно собирать и тестировать по отдельности, а что вынуждено тянет за собой всё остальное при любой сборке.

Прямая связь с DI, который разбирали раньше: Dependency Injection — это техника ("как передать зависимость" — через конструктор), а Dependency Inversion — это принцип ("кому принадлежит абстракция и в какую сторону смотрит зависимость"). В примере `main.cpp` — composition root — использует DI (`OrderProcessor processor(gateway)`), чтобы связать оба модуля, но сама возможность независимой сборки domain — заслуга DIP, а не DI. Можно делать DI без DIP (передавать через конструктор конкретный `StripeGateway*`, без всякого интерфейса — DI есть, инверсии нет) и в теории DIP без DI (например, через глобальную регистрацию фабрики) — это разные оси одной проблемы.
