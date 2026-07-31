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
    // что и для ЛЮБОГО другого способа создать Order. Обойти инвариант,
    // придя "снаружи" через DTO, невозможно - fromDTO() не имеет доступа
    // ни к чему, кроме публичного конструктора Order.
}

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