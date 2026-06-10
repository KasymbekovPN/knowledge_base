#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

struct Account {
    int balance{};
    std::mutex mtx;

    Account(const int _balance): balance{_balance} {}
};

void transfer_ab(Account& _a, Account& _b, int amount) {
    std::scoped_lock lock{_a.mtx, _b.mtx};
    _a.balance -= amount;
    _b.balance += amount;
}

void transfer_ba(Account& _a, Account& _b, int amount) {
    std::scoped_lock lock{_b.mtx, _a.mtx};
    _b.balance -= amount;
    _a.balance += amount;
}

int main() {
    Account alice{1000};
    Account bob{900};

    std::vector<std::thread> threads;
    for (size_t i{}; i < 50; ++i) {
        threads.emplace_back(std::thread(transfer_ab, std::ref(alice), std::ref(bob), 1));
        threads.emplace_back(std::thread(transfer_ba, std::ref(alice), std::ref(bob), 2));
    }

    for (auto &&t: threads) {
        t.join();
    }

    std::cout << "Alice " << alice.balance << std::endl;
    std::cout << "Bob " << bob.balance << std::endl;

    return 0;
}
