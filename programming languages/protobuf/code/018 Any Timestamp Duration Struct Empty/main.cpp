#include <iostream>
#include <format>

#include <google/protobuf/any.pb.h>
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/duration.pb.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/empty.pb.h>
#include <google/protobuf/util/time_util.h>

#include "user.pb.h"

int main() {
    // ===== Any: контейнер для сообщения ЛЮБОГО типа + строка типа =====
    {
        myapp::User user;
        user.set_id(7);
        user.set_name("Alice");

        google::protobuf::Any any;
        // сериализует User внутрь Any + запоминает type_url
        any.PackFrom(user);

        std::cout << "=== Any ===\n";
        std::cout << std::format("type_url: {}\n", any.type_url());
        std::cout << std::format("Is<myapp::User>(): {}\n", any.Is<myapp::User>());

        myapp::User unpacked;
        bool ok{any.UnpackTo(&unpacked)};
        std::cout << std::format("UnpackTo ok: {}, name: {}\n\n", ok, unpacked.name());
    }

    // ===== Timestamp: точка во времени (секунды + наносекунды от эпохи) =====
    {
        google::protobuf::Timestamp ts{google::protobuf::util::TimeUtil::GetCurrentTime()};
        std::cout << "=== Timestamp ===\n";
        std::cout << std::format("seconds: {}, nanos: {}\n", ts.seconds(), ts.nanos());
        std::cout << std::format("like ISO8601: {}\n", google::protobuf::util::TimeUtil::ToString(ts));

        // из time_t в Timestamp и обратно
        google::protobuf::Timestamp fixed{google::protobuf::util::TimeUtil::SecondsToTimestamp(1700000000)};
        std::cout << std::format("fixed timestamp: {}\n\n", google::protobuf::util::TimeUtil::ToString(fixed));
    }

    // ===== Duration: промежуток времени (может быть отрицательным) =====
    {
        google::protobuf::Duration d{google::protobuf::util::TimeUtil::SecondsToDuration(90)};
        std::cout << "=== Duration ===\n";
        std::cout << std::format("90 seconds -> {}, like a string {}\n", d.seconds(), google::protobuf::util::TimeUtil::ToString(d));
        google::protobuf::Duration neg{google::protobuf::util::TimeUtil::SecondsToDuration(-30)};
        std::cout << std::format("-30 seconds -> {}\n\n", neg.seconds());
    }

    // ===== Struct: динамическая, нетипизированная структура (как JSON-объект) =====
    {
        google::protobuf::Struct s;
        auto& fields = *s.mutable_fields();

        fields["name"].set_string_value("Alice");
        fields["age"].set_number_value(30);
        fields["is_admin"].set_bool_value(false);

        google::protobuf::Value* tags = &fields["tags"];
        tags->mutable_list_value()->add_values()->set_string_value("vip");
        tags->mutable_list_value()->add_values()->set_string_value("beta");

        std::cout << "=== Struct (dynamic schema, without .proto) ===\n";
        std::cout << s.DebugString();
    }

    // ===== Empty: маркер "нет данных", часто как ответ RPC =====
    {
        google::protobuf::Empty e;
        std::string bytes;
        e.SerializeToString(&bytes);
        std::cout << "=== Empty ===\n";
        std::cout << "serialized size = " << bytes.size() << " bytes (always 0)\n";
    }

    return 0;
}
