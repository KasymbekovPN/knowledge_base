
#include <iostream>
#include "user.pb.h"

int main() {
    myapp::User u;

    // --- геттер/сеттер для скаляра ---
    u.set_id(42);
      std::cout << "id = " << u.id() << "\n";

    // --- сеттер/геттер для string ---
    u.set_name("Alice");
    std::cout << "name = " << u.name() << "\n";

    // --- mutable_ для string: изменение "на месте" без промежуточной копии ---
    u.mutable_name()->append(" Smith");
    std::cout << "name after mutable_append = " << u.name() << "\n";

    // --- optional (proto3): has_ появляется только благодаря `optional` ---
    std::cout << "has apartment (Address) before set: n/a — optional только у Address.apartment\n";

    // --- repeated message: add_ возвращает указатель на новый элемент ---
    myapp::Address* addr1 = u.add_addresses();
    addr1->set_city("Berlin");
    addr1->set_street("Alexanderplatz 1");

    myapp::Address* addr2 = u.add_addresses();
    addr2->set_city("Munich");

    std::cout << "addresses_size = " << u.addresses_size() << "\n";
    for (int i = 0; i < u.addresses_size(); ++i) {
    std::cout << "  address[" << i << "].city = " << u.addresses(i).city() << "\n";
    }

    // --- optional на скаляре: has_ метод ---
    addr1->set_apartment("12B");
    std::cout << "addr1.has_apartment() = " << addr1->has_apartment() << "\n";
    std::cout << "addr2.has_apartment() = " << addr2->has_apartment() << " (не установлено)\n";

    // --- map: mutable_ возвращает Map<K,V>&, работаем как с std::map ---
    (*u.mutable_preferences())["theme"] = "dark";
    (*u.mutable_preferences())["locale"] = "en_US";
    std::cout << "preferences[theme] = " << u.preferences().at("theme") << "\n";

    // --- oneof: set_ на одном из полей автоматически "гасит" другое ---
    u.set_phone("+491234567");
    std::cout << "contact_method_case == kPhone? "
            << (u.contact_method_case() == myapp::User::kPhone) << "\n";

    u.set_telegram_handle("@alice");
    std::cout << "after set_telegram_handle:\n";
    std::cout << "  contact_method_case == kPhone? "
            << (u.contact_method_case() == myapp::User::kPhone) << "\n";
    std::cout << "  contact_method_case == kTelegramHandle? "
            << (u.contact_method_case() == myapp::User::kTelegramHandle) << "\n";
    std::cout << "  phone field is now empty: \"" << u.phone() << "\"\n";

    // --- сериализация, чтобы доказать, что объект валиден целиком ---
    std::string bytes;
    u.SerializeToString(&bytes);
    std::cout << "serialized size = " << bytes.size() << " bytes\n";

    myapp::User parsed;
    parsed.ParseFromString(bytes);
    std::cout << "round-trip name = " << parsed.name() << ", addresses = "
            << parsed.addresses_size() << "\n";

    return 0;
}

/*

protoc -I. --cpp_out=../gen common/address.proto users/user.proto orders/order.proto

*/