#include <iostream>
#include <mutex>

struct Account {
    std::mutex mtx;
    int balance;

    Account(const int _balance): balance{_balance} {}
};

std::ostream& operator<<(std::ostream& _os, const Account& _account) {
    return _os << "{" << _account.balance << "}";
}

void safe_transfer(Account& _a, Account& _b, const int amount) {
    std::lock(_a.mtx, _b.mtx);

    std::lock_guard lock1(_a.mtx, std::adopt_lock); // take ownship
    std::lock_guard lock2(_b.mtx, std::adopt_lock); // without lock

    _a.balance -= amount;
    _b.balance += amount;
}

int main() {
    Account a{100};
    Account b{200};

    safe_transfer(a, b, 50);

    std::cout << "A: " << a << "\n";
    std::cout << "B: " << b << "\n";

    return 0;
}
