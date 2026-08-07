#include <iostream>
#include "schema_v1.pb.h"
#include "schema_v2_good.pb.h"
#include "schema_v2_bad.pb.h"

int main() {
    // ===== Сценарий 1: старый клиент (v1) шлёт данные =====
    myapp::v1::Event v1;
    v1.set_id(100);
    v1.set_type("click");
    v1.set_retry_count(5);

    std::string bytes;
    v1.SerializeToString(&bytes);
    std::cout << "--- v1 sent ---\n" << v1.Utf8DebugString() << "\n";

    // --- Безопасная эволюция: новый сервер (v2good) читает старые данные ---
    myapp::v2good::Event good;
    good.ParseFromString(bytes);
    std::cout << "--- v2good read (safe schema) ---\n"
             << good.Utf8DebugString();
    std::cout << "email (new field is absent) = \""
             << good.email() << "\"\n";
    const auto& unknown =
      good.GetReflection()->GetUnknownFields(good);
    std::cout << "unknown fields count = " << unknown.field_count()
             << "  <- bites retry_count saved as unknown, did not lose\n\n";

    // --- ОПАСНАЯ эволюция: сервер v2bad переиспользовал номер 3 ---
    myapp::v2bad::Event bad;
    bad.ParseFromString(bytes);
    std::cout << "--- v2bad read (unsafe schema, number 3 reused) ---\n"
             << bad.Utf8DebugString();
    std::cout << "priority = " << bad.priority()
        << "  <- This is actually the retry_count=5 from the old client!\n"
        << "   There is no error: the wire_type for int32 retry_count and int32\n"
        << "   priority is the same, so the parser silently assigned bytes\n"
        << "   intended for the other field to the new one.\n\n";

    // ===== Сценарий 2: новый клиент (v2good) шлёт данные со старым сервером =====
    myapp::v2good::Event g2;
    g2.set_id(200);
    g2.set_type("purchase");
    g2.set_email("a@b.com");

    std::string bytes2;
    g2.SerializeToString(&bytes2);
    std::cout << "--- v2good sent (with new field email) ---\n"
             << g2.Utf8DebugString() << "\n";

    myapp::v1::Event old;
    bool ok = old.ParseFromString(bytes2);
    std::cout << "--- v1 (old parser) read ---\n"
             << "ParseFromString ok=" << ok << "\n"
             << old.Utf8DebugString()
             << "retry_count (field email is absent in v1) = "
             << old.retry_count() << " (default, did not fail)\n"
             << "  The old client doesn't crash, but it doesn't see anything email — forward compatibility.\n";

    return 0;
}