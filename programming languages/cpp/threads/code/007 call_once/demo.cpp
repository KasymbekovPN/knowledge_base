#include <iostream>
#include <thread>
#include <mutex>
#include <memory>

struct Database {
public:
    static Database& instance() {
        std::call_once(init_flag, []() {
            db = std::make_unique<Database>(Database());
        });
        return *db;
    }

    void query() {
        std::cout << "Query execution..." << std::endl;
    }

private:
    Database() {
        std::cout << "DB connected" << std::endl;
    }

    static std::once_flag init_flag;
    static std::unique_ptr<Database> db;
};

std::once_flag Database::init_flag;
std::unique_ptr<Database> Database::db;

void call_worker() {
    Database::instance().query();
}

int main() {
    std::thread t0 {call_worker};
    std::thread t1 {call_worker};

    t0.join();
    t1.join();

    return 0;
}
