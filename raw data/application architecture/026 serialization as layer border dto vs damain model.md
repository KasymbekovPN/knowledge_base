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
// DTO (data transfer object) vs доменная модель: сериализация - граница, а не сквозной тип.  
// DTO - плоская, "глупая" структура под конкретный wire-формат.  
// Order - доменная модель с инвариантами, которая вообще не знает о существовании  
// DTO/JSON/API. Между ними - явный, единственный слой маппинга.  
  
#include <iostream>  
#include <format>  
#include <stdexcept>  
#include <string>  
  
// ---------------------------------------------------------------------------  
// DTO - 1:1 с "форматом на проводе" (в реальности - JSON/protobuf-схема,  
// тут для наглядности - обычный struct). Публичные поля, никакой логики,  
// никаких инвариантов - чистая транспортная форма данных.  
// Имя поля на проводе НАРОЧНО отличается от домена.  
// wire хранит цену в центах - типично для платёжных API  
// ---------------------------------------------------------------------------  
namespace {  
    struct OrderDTO {  
        std::string order_id;  
        std::string customer_email;  
        int item_quantity;  
        double unit_price_cents;  
    };
}  
  
// ---------------------------------------------------------------------------  
// Доменная модель - инкапсулирует инварианты ("quantity > 0", "price >= 0"),  
// хранит цену как доллары (внутреннее представление, никак не обязанное  
// совпадать с wire-форматом). Не включает НИ ОДНОГО заголовка про  
// сериализацию, JSON или что угодно транспортное.  
// ---------------------------------------------------------------------------  
class Order {  
public:  
    Order(std::string id, std::string customer, const int quantity, const double unitPriceDollars):  
        id_{std::move(id)},  
        customer_{std::move(customer)},  
        quantity_{quantity},  
        unitPrice_{unitPriceDollars} {  
  
        if (quantity_ <= 0) throw std::invalid_argument{"quantity must be positive"};  
        if (unitPrice_ < 0) throw std::invalid_argument{"unitPrice must be greater than zero"};  
    }  
    double total() const { return quantity_ * unitPrice_; }  
    const std::string& id() const { return customer_; }  
    const std::string& customer() const { return customer_; }  
    int quantity() const { return quantity_; }  
    double unitPrice() const { return unitPrice_; }  
private:  
    std::string id_;  
    std::string customer_;  
    int quantity_{};  
    double unitPrice_{};  
};  
  
// ---------------------------------------------------------------------------  
// Маппинг - единственное место, знающее и про DTO, и про Order.  
// ---------------------------------------------------------------------------  
static Order fromDTO(const OrderDTO& dto) {  
    // конвертация формата - забота границы  
    return {  
        dto.order_id,  
        dto.customer_email,  
        dto.item_quantity,  
        dto.unit_price_cents / 100.0  
    };
  
    // Order() бросит исключение при невалидных данных - тот же путь валидации,  
    // что и для ЛЮБОГО другого способа создать Order. Обойти инвариант,    // придя "снаружи" через DTO, невозможно - fromDTO() не имеет доступа    // ни к чему, кроме публичного конструктора Order.}  
  
static OrderDTO toDTO(const Order& order) {  
    OrderDTO dto;  
    dto.order_id = order.id();  
    dto.customer_email = order.customer();  
    dto.item_quantity = order.quantity();  
    dto.unit_price_cents = order.unitPrice() * 100.0;  
  
    return dto;  
}  
  
int main() {  
    std::cout << "-- valid DTO -> Order --\n";  
    const OrderDTO validDto{  
        .order_id = "ORD-1",  
        .customer_email = "pablo@example.com",  
        .item_quantity = 3,  
        .unit_price_cents = 1999.0 };  // $19.99 в центах  
    const Order order = fromDTO(validDto);  
    std::cout << "  order.total() = $" << order.total() << "\n";  
  
    std::cout << "\n-- Order -> DTO --\n";  
    const OrderDTO backToDto = toDTO(order);  
    std::cout << "  dto.unit_price_cents = " << backToDto.unit_price_cents << "\n";  
  
    std::cout << "\n-- invalid DTO (bad payload) --\n";  
    try {  
        const OrderDTO badDto{  
            .order_id = "ORD-2",  
            .customer_email = "hacker@example.com",  
            .item_quantity = -5,  
            .unit_price_cents = 1000.0};  
        const Order bad = fromDTO(badDto);  
        std::cout << "  НЕОЖИДАННО создался: " << bad.total() << "\n";  
    } catch (const std::invalid_argument& e) {  
        std::cout << "  fromDTO() canceled invalid data: " << e.what() << "\n";  
    }  
    return 0;  
}
```

**Что демонстрирует расхождение имён и форматов**

`OrderDTO.customer_email` vs `Order::customer()`, `item_quantity` vs `quantity()`, а главное — `unit_price_cents` (центы, snake_case, типичное API-соглашение) vs `unitPrice_` (доллары, внутреннее представление). Это расхождение не случайность и не небрежность — это доказательство того, что домен и wire-формат **независимо эволюционируют**. `fromDTO()`/`toDTO()` — единственное место во всей программе, которое знает про оба представления сразу; ни `Order`, ни то, что вызывает `Order`-методы где-то в бизнес-логике, вообще не подозревают, что где-то на границе цена хранится в центах, а не в долларах.

**Почему нельзя просто "навесить JSON-теги на доменный класс"**

Соблазн — сделать один класс сразу и доменным, и сериализуемым (аннотировать поля для JSON-библиотеки прямо в `Order`). Проблема в двух направлениях сразу. Если формат API меняется (переименовали поле для нового клиента, добавили версионирование, как обсуждали в теме versioning интерфейсов) — приходится трогать файл с бизнес-логикой ради чисто транспортного изменения. Если бизнес-логика меняется (переименовали внутреннее понятие для ясности, изменили единицы измерения) — это неожиданно ломает контракт API для внешних потребителей, которые ничего не просили менять. Это ровно DIP в новой форме: доменный слой — высокоуровневый, формат сериализации — низкоуровневая деталь, и зависимость должна идти от низкоуровневого к высокоуровневому (маппинг знает про Order, Order не знает про DTO), а не наоборот.

**Главная опасность — обход инварианта через десериализацию**

Многие сериализационные библиотеки создают объект не через конструктор, а через reflection/friend-доступ, устанавливая приватные поля напрямую в обход всей валидирующей логики — это тот же класс проблемы, что был в демо про протекающую абстракцию с мутабельным геттером: путь, который выглядит как "просто загрузка данных", на деле оказывается ещё одним способом мутировать состояние в обход правил. Тест это прямо проверяет: `badDto` с отрицательным `item_quantity` (представь — испорченный или намеренно подделанный payload из сети) проходит через `fromDTO()`, который вызывает **тот же самый** валидирующий конструктор `Order(...)`, что и любой другой путь создания заказа — и получает `invalid_argument`, а не тихо создаёт заказ с отрицательным количеством товара. Домен защищён от плохих внешних данных ровно потому, что единственный вход в него — публичный конструктор с проверками, а не reflection-based заполнение приватных полей.

**Множественность DTO — тоже часть паттерна**

В реальном проекте одному `Order` может соответствовать несколько разных DTO — то, что уходит во внешний REST API, то, что хранится в БД, то, что летит во внутреннем событии по шине (как `TaskAdded`/`OrderPlaced` из наших примеров с `EventBus`). Если бы всё это был один тип, он оказался бы вынужден одновременно удовлетворять форме трёх разных, потенциально конфликтующих контрактов — снова то же нарушение ISP, которое разбирали раньше, только применительно не к интерфейсам поведения, а к формам данных.
