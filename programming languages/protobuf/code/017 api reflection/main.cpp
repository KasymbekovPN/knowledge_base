#include <iostream>
#include <string>
#include <format>
#include <vector>
#include <unordered_map>
#include <functional>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "user.pb.h"
#include "order.pb.h"

namespace {

    void printSetFields(const google::protobuf::Message& msg, const int indent = 0);

    std::unordered_map<
        google::protobuf::FieldDescriptor::CppType,
        std::function<bool(
            const google::protobuf::Reflection*,
            const google::protobuf::Message&,
            const google::protobuf::FieldDescriptor*,
            const int)>> HANDLERS = {

        {
            google::protobuf::FieldDescriptor::CPPTYPE_INT32,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetInt32(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_INT64,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetInt64(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetDouble(msg, field);
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_BOOL,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetBool(msg, field) ? "true" : "false";
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_STRING,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << std::format("\"{}\"", reflection->GetString(msg, field));
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_ENUM,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << reflection->GetEnum(msg, field)->name();
                return false;
            }
        },
        {
            google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE,
            [](
                const google::protobuf::Reflection* reflection,
                const google::protobuf::Message& msg,
                const google::protobuf::FieldDescriptor* field,
                const int indent) {

                std::cout << '\n';
                printSetFields(reflection->GetMessage(msg, field), indent + 2);
                return true;
            }
        }
    };

    const auto DEFAULT_HANDLER = [](const google::protobuf::Reflection*,
                                    const google::protobuf::Message&,
                                    const google::protobuf::FieldDescriptor*,
                                    const int) {
        std::cout << "<NOT HANDLED IN DEMO>\n";
        return false;
    };

    // Полностью generic-функция: не знает ни про User, ни про Order,
    // ни про какой-либо другой конкретный тип сообщения. Работает через
    // базовый класс google::protobuf::Message + Reflection + Descriptor.
    void printSetFields(const google::protobuf::Message& msg, const int indent) {
        const google::protobuf::Descriptor* descriptor{msg.GetDescriptor()};
        const google::protobuf::Reflection* reflection{msg.GetReflection()};
        std::string pad(indent * 2, ' ');

        std::cout << std::format("{}{} {{\n", pad, descriptor->name());

        // ListFields() возвращает только УСТАНОВЛЕННЫЕ поля (в отличие от
        // перебора всех field_count() дескриптора, где пришлось бы руками
        // звать HasField для каждого)
        std::vector<const google::protobuf::FieldDescriptor*> fields;
        reflection->ListFields(msg, &fields);

        for (const auto* field: fields) {
            std::cout << std::format("{} {} (#{}, {}) =", pad, field->name(), field->number(), field->type_name());

            if (field->is_repeated()) {
                const int count{reflection->FieldSize(msg, field)};
                std::cout << std::format("[{} items]\n", count);
                if (field->cpp_type() ==
                    google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
                    // рекурсивно печатаем первые пару вложенных сообщений
                    for (int i{}; i < std::min(count, 2); ++i) {
                        printSetFields(reflection->GetRepeatedMessage(msg, field, i), indent + 2);
                    }
                }
                continue;
            }

            auto handler = HANDLERS.contains(field->cpp_type()) ? HANDLERS[field->cpp_type()] : DEFAULT_HANDLER;
            if (handler(reflection, msg, field, indent)) continue;
            std::cout << '\n';
        }
        std::cout << std::format("{}\n", pad);
    }

    // Ещё один generic-инструмент: подсчёт установленных полей рекурсивно,
    // работает для ЛЮБОГО типа сообщения без единой строчки типоспецифичного кода.
    int countSetFieldsRecursive(const google::protobuf::Message& msg) {
        const google::protobuf::Reflection* reflection{msg.GetReflection()};
        std::vector<const google::protobuf::FieldDescriptor*> fields;
        reflection->ListFields(msg, &fields);

        int total{static_cast<int>(fields.size())};
        for (const auto* field: fields) {
            if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
                if (field->is_repeated()) {
                    const int count{reflection->FieldSize(msg, field)};
                    for (int i{}; i < count; ++i) {
                        total += countSetFieldsRecursive(reflection->GetRepeatedMessage(msg, field, i));
                    }
                } else {
                    total += countSetFieldsRecursive(reflection->GetMessage(msg, field));
                }
            }
        }

        return total;
    }

}

int main() {
    myapp::User user;
    user.set_id(1);
    user.set_name("Alice");
    user.set_phone("+123");
    user.add_addresses()->set_city("Berlin");
    user.add_addresses()->set_city("Munich");

    myapp::Order order;
    order.set_order_id(42);
    order.set_total_amount(199.5);
    order.set_status(myapp::Order::ORDER_STATUS_PAID);
    order.add_items()->set_sku("A1");
    order.add_items()->set_sku("A2");

    std::cout << "=== PrintSetFields(user) - type defined in runtime ===\n";
    printSetFields(user);

    std::cout << "\n=== PrintSetFields(order) - same func other type ===\n";
    printSetFields(order);

    std::cout << "\n=== CountSetFieldsRecursive ===\n";
    std::cout << "user:  " << countSetFieldsRecursive(user) << " set fields (with nested)\n";
    std::cout << "order: " << countSetFieldsRecursive(order) << " set fields (with nested)\n";

    return 0;
}
