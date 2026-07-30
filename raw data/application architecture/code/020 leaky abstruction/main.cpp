// Протекающая абстракция (leaky abstraction): интерфейс формально скрывает
// реализацию, но детали конкретного backend'а всё равно "просачиваются"
// наружу - через тип исключения или через то, что "геттер" на самом деле
// отдаёт мутабельную ссылку на внутреннее состояние.

#include <iostream>
#include <format>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <ranges>

struct User { int id; std::string name; };

// ============================================================
// Вариант 1 (протекающая абстракция)
// ============================================================

namespace leaky {

    namespace {
        class IUserRepository {
        public:
            virtual ~IUserRepository() = default;
            // какое исключение бросит при "не найдено"? не определено контрактом
            virtual User findById(int id) = 0;
            // "геттер", отдающий мутабельную ссылку на внутреннее хранилище
            virtual std::vector<User>& getAllMutable() = 0;
        };
    }

    namespace {
        // "SQL"-подобная реализация - симулируем типичную для конкретной БД ошибку
        class SqlNotFoundError: public std::runtime_error {
        public:
            explicit SqlNotFoundError(const int id):
                std::runtime_error(std::format("SQL: row not found, id = {}", id)) {}
        };
    }

    namespace {
        class SqlUserRepository: public IUserRepository {
        public:
            SqlUserRepository() {
                users_ = {{.id = 1, .name = "Pablo"}, {.id = 2, .name = "Anna"}};
            }

            User findById(const int id) override {
                for (const auto& user : users_) if (user.id == id) return user;
                // деталь конкретного backend'а "утекла" в публичный контракт
                throw SqlNotFoundError(id);
            }

            std::vector<User>& getAllMutable() override { return users_; }

        private:
            std::vector<User> users_;
        };
    }

    namespace {
        class InMemoryUserRepository: public IUserRepository {
        public:
            User findById(const int id) override {
                const auto IT{users_.find(id)};
                // ДРУГОЙ тип исключения!
                if (IT == users_.end()) throw std::out_of_range(std::format("id not found = {}", id));

                return IT->second;
            }

            std::vector<User>& getAllMutable() override {
                cache_.clear();
                for (const auto& u: users_ | std::views::values) cache_.push_back(u);
                // мутация этой ссылки НЕ трогает users_ - поведение расходится с SqlUserRepository
                return cache_;
            }

        private:
            std::map<int, User> users_ = {{1, {.id = 1, .name = "Pablo"}}};
            std::vector<User> cache_;
        };
    }

    static void demo() {
        std::cout << "--- leaky ---\n";

        // Проблема 1: клиентский код, написанный и протестированный против SQL...
        std::unique_ptr<IUserRepository> repo = std::make_unique<SqlUserRepository>();
        try {
            repo->findById(999);
        } catch (const SqlNotFoundError& e) {
            std::cout << std::format("  [sql] caught waited exception: {}\n", e.what());
        }

        // ...а теперь backend поменяли на InMemory - тот же catch-блок молча перестаёт ловить
        repo = std::make_unique<InMemoryUserRepository>();
        try {
            repo->findById(999);
        } catch (const SqlNotFoundError& e) {
            std::cout << std::format("  [InMemory] caught (SQL): \n", e.what());
        } catch (const std::exception& e) {
            std::cout << std::format("  [InMemory] did not catch wiated type: {}\n", e.what());
        }

        // Проблема 2: "просто геттер" на деле отдаёт мутабельную ссылку на источник правды
        repo = std::make_unique<SqlUserRepository>();
        std::cout << std::format("  [sql] size before: {}\n", repo->getAllMutable().size());
        // выглядит как безобидное чтение, а на деле - мутация состояния
        repo->getAllMutable().clear();
        std::cout << std::format("  [sql] size after getAllMutable().clear(): {}\n", repo->getAllMutable().size());
    }

}

// ============================================================
// Вариант 2: интерфейс сам объявляет доменный контракт ошибок и владения
// ============================================================
namespace clean {
    namespace {
        // Интерфейс объявляет СВОЁ исключение - любая реализация обязана
        // переводить в него специфичные для backend'а ошибки на границе.
        class UserNotFoundException: public std::runtime_error {
        public:
            explicit UserNotFoundException(const int id):
                std::runtime_error(std::format("User not found id: = {}", id)) {}
        };
    }

    namespace {
        class IUserRepository {
        public:
            virtual ~IUserRepository() = default;
            // контракт: бросает ТОЛЬКО UserNotFoundException
            virtual User findById(int id) = 0;
            // ПО ЗНАЧЕНИЮ - внутреннее хранилище недостижимо снаружи
            virtual std::vector<User> getAll() const = 0;
        };
    }

    namespace {
        class SqlUserRepository: public IUserRepository {
        public:
            SqlUserRepository() { users_ = {{.id = 1, .name = "Pablo"}, {.id = 2, .name = "Anna"}}; }
            User findById(const int id) override {
                for (auto& u: users_) if (u.id == id) return u;
                // SQL-специфичная ошибка переведена в доменную ЗДЕСЬ, на границе
                throw UserNotFoundException(id);
            }

            // copy
            std::vector<User> getAll() const override { return users_; }

        private:
            std::vector<User> users_;
        };
    }

    namespace {
        class InMemoryUserRepository: public IUserRepository {
        public:
            User findById(const int id) override {
                const auto IT{users_.find(id)};
                if (IT == users_.end()) throw UserNotFoundException(id);

                return IT->second;
            }

            std::vector<User> getAll() const override {
                std::vector<User> result;
                for (const auto& u: users_ | std::views::values) result.push_back(u);

                return result;
            }

        private:
            std::map<int, User> users_ = {{1, {.id = 1, .name = "Pablo"}}};
        };
    }

    static void testFindNotFound(const std::string& label, const std::unique_ptr<IUserRepository>& repo) {
        try {
            repo->findById(999);
        } catch (const UserNotFoundException& e) {
            std::cout << std::format("  [{}] caught (one type, independently form backend): {}\n", label, e.what());
        }
    }

    static void demo() {
        std::cout << "--- clean ---\n";

        testFindNotFound("Sql", std::make_unique<SqlUserRepository>());
        testFindNotFound("InMemory", std::make_unique<InMemoryUserRepository>());

        const auto repo{std::make_unique<SqlUserRepository>()};
        auto users{repo->getAll()};
        // это ЛОКАЛЬНАЯ копия - внутреннее состояние repo не тронуто
        users.clear();
        std::cout << std::format("  repo->getAll().size() = {}\n", repo->getAll().size());
    }

}

int main() {
    leaky::demo();
    clean::demo();

    return 0;
}