#include <fstream>
#include <iostream>
#include <format>

#include "users/user.pb.h"

int main() {
    const std::string path = "user.bin";

    // --- 1. Создаём сообщение ---
    myapp::User original;
    original.set_id(101);
    original.set_name("Bob");
    original.set_email("bob@example.com");
    original.add_addresses()->set_city("Paris");
    original.set_phone("+33123456789");

    std::cout << "Message created:\n" << original.Utf8DebugString() << "\n";

    // --- 2. Сериализуем в файл ---
    {
        std::ofstream out(path, std::ios::binary);
        if (!original.SerializeToOstream(&out)) {
            std::cerr << "serialization error in file\n";
            return 1;
        }
    }
    std::cout << "write to file: " << path << "\n";

    // --- 3. Читаем обратно из файла ---
    myapp::User loaded;
    {
        std::ifstream in(path, std::ios::binary);
        if (!loaded.ParseFromIstream(&in)) {
            std::cerr << "file reading error\n";
            return 1;
        }
    }

    std::cout << "\nRead form file:\n" << loaded.Utf8DebugString() << "\n";

    // --- 4. Проверка round-trip ---
    bool matches = (original.SerializeAsString() == loaded.SerializeAsString());
    std::cout << "Round-trip matches: " << (matches ? "Yes" : "NO") << "\n";

    return matches ? 0 : 1;
}
