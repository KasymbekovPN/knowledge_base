#include <iostream>
#include <mutex>

// Макросы-обёртки — обычно именно так это и используют на практике
// (см. abseil mutex.h), потому что голый __attribute__ громоздкий
#define GUARDED_BY(x) __attribute__((guarded_by(x)))
#define REQUIRES(...) __attribute__((requires_capability(__VA_ARGS__)))
#define ACQUIRE(...) __attribute__((acquire_capability(__VA_ARGS__)))
#define RELEASE(...) __attribute__((release_capability(__VA_ARGS__)))
#define CAPABILITY(x) __attribute__((capability(x)))
#define SCOPED_CAPABILITY __attribute__((scoped_lockable))
#define NO_THREAD_SAFETY_ANALYSIS __attribute__((no_thread_safety_analysis))

namespace demo0 {
    class BankAccount {
    public:
        void deposit(const int amount) {
            std::lock_guard<std::mutex> lock{mu_};
            // ok — mu_ захвачен через lock_guard
            balance_ += amount;
        }

        // ОШИБКА КОМПИЛЯЦИИ: обращение к balance_ без захвата mu_
        int getBalanceUnsafe() const {
            return balance_;
        }

    private:
        std::mutex mu_;
        // balance_ можно трогать только под mu_
        int balance_ GUARDED_BY(mu_) = 0;
    };
}

namespace demo1 {

    class CAPABILITY("mutex") Mutex {
    public:
        void Lock() ACQUIRE() {}
        void Unlock() RELEASE() {}
    };

    class SCOPED_CAPABILITY MutexLock {
    public:
        explicit MutexLock(Mutex* mu) ACQUIRE(mu): mu_{mu} {
            mu_->Lock();
        }
        ~MutexLock() RELEASE() { mu_->Unlock(); }

    private:
        Mutex* mu_{nullptr};
    };

    class BankAccount {
    public:
        // ok
        void deposit(const int amount) {
            MutexLock lock{&mu_};
            balance_ += amount;
        }

        // ошибка: чтение guarded_by-поля без лока
        int getBalanceUnsafe() const {
            return balance_;
        }

        // ok — вызывающий обязан уже держать mu_
        void requireLocked() REQUIRES(mu_) {
            balance_ += 1;
        }

    private:
        Mutex mu_;
        int balance_ GUARDED_BY(mu_) = 0;
    };

}

int main() {
    auto ba0 = demo0::BankAccount();
    ba0.deposit(10);
    std::cout << ba0.getBalanceUnsafe() << std::endl;

    auto ba1 = demo1::BankAccount();
    ba1.deposit(10);
    std::cout << ba1.getBalanceUnsafe() << std::endl;

    return 0;
}

/*

clang++.exe -Wthread-safety -std=c++23 -c main.cpp

*/
