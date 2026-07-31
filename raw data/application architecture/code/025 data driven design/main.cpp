// Data-Driven design: сравниваем хардкод (новый тип врага = новая ветка
// кода + пересборка) с данными из внешнего файла (новый тип врага =
// правка enemies.csv, УЖЕ СКОМПИЛИРОВАННЫЙ бинарник подхватывает его
// без единой пересборки - это и проверяется в самом конце).

#include <sstream>
#include <fstream>
#include <iostream>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    struct Enemy {
        std::string name;
        int health;
        int damage;
        float speed;
    };
}

// ============================================================
// Вариант 1 (хардкод): каждый новый тип врага - новая ветка в коде.
// ============================================================
namespace hardcoded {
    namespace {
        enum class EnemyType { Goblin, Orc, Dragon };
    }

    static Enemy createEnemy(const EnemyType type) {
        switch (type) {
            case EnemyType::Goblin: return {.name = "Goblin", .health = 20, .damage = 3, .speed = 1.5f};
            case EnemyType::Orc: return {.name = "Orc", .health = 50, .damage = 8, .speed = 1.0f};
            case EnemyType::Dragon: return {.name = "Dragon", .health = 500, .damage = 40, .speed = 0.8f};
        }
        throw std::runtime_error("unknown enemy type");
        // Чтобы добавить Trol - нужно: (1) добавить значение в enum,
        // (2) добавить case в switch, (3) пересобрать программу. Баланс-дизайнер
        // без доступа к компилятору и репозиторию этого сделать не может.
    }

    static void demo() {
        std::cout << "-- hardcoded --\n";
        for (const auto type: {EnemyType::Goblin, EnemyType::Orc, EnemyType::Dragon}) {
            const auto [name, health, damage, speed]{createEnemy(type)};
            std::cout << std::format("  {}: hp={}, dmg={}, speed={}\n", name, health, damage, speed);
        }
    }

}

// ============================================================
// Вариант 2 (data-driven): враги - строки в CSV, движок ("спаунер")
// написан ОДИН РАЗ и не знает заранее, сколько типов врагов будет
// существовать и какими они окажутся.
// ============================================================
namespace data_driven {
    namespace {
        class EnemyDatabase {
        public:
            // Загружает определения из внешнего файла - это НЕ часть исходного
            // кода программы, файл можно менять независимо от бинарника.
            static EnemyDatabase loadFromCsv(const std::string& path) {
                EnemyDatabase db;
                std::ifstream file{path};
                if (!file) throw std::runtime_error(std::format("cannot open file '{}'", path));

                std::string line;
                // пропускаем заголовок "name,health,damage,speed"
                std::getline(file, line);

                while(std::getline(file, line)) {
                    if (line.empty()) continue;
                    std::istringstream ss(line);
                    std::string name, health, damage, speed;
                    std::getline(ss, name, ',');
                    std::getline(ss, health, ',');
                    std::getline(ss, damage, ',');
                    std::getline(ss, speed, ',');
                    db.definitions_.push_back({
                        .name = name,
                        .health = std::stoi(health),
                        .damage = std::stoi(damage),
                        .speed = std::stof(speed)});
                }

                return db;
            }

            Enemy spawn(const std::string& name) const {
                for (const auto& def: definitions_) if (def.name == name) return def;
                throw std::runtime_error(std::format("unknown enemy: {}\n", name));
            }

            const std::vector<Enemy>& all() const { return definitions_; }

        private:
            std::vector<Enemy> definitions_;
        };
    }

    static void demo(const std::string& csvPath) {
        std::cout << std::format("\n-- data_driven (load from '{}')---\n", csvPath);
        const auto db{EnemyDatabase::loadFromCsv(csvPath)};
        for (const auto& [name, health, damage, speed]: db.all()) {
            std::cout << std::format("  {}: hp={}, dmg={}, speed={}\n", name, health, damage, speed);
        }
    }

}

int main(const int argc, char** argv) {
    hardcoded::demo();

    const std::string CSV_PATH{
        (argc > 1)
        ? argv[1]
        : "C:\\projects\\knowledge_base\\raw data\\application architecture\\code\\025 data driven design\\enemies.csv"
    };
    data_driven::demo(CSV_PATH);

    return 0;
}
