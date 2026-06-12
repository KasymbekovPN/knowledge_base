#include <iostream>
#include <mutex>

using namespace std;

class Counter {
private:
    mutable recursive_mutex mtx;
    int count {0};

public:
    void inc() {
        lock_guard<recursive_mutex> lock(mtx);
        ++count;
    }

    void inc2() {
        lock_guard<recursive_mutex> lock(mtx);
        inc();
        inc();
    }

    void reset() {
        lock_guard<recursive_mutex> lock(mtx);
        count = 0;
    }

    int get() const {
        lock_guard<recursive_mutex> lock(mtx);
        return count;
    }
};

int main() {
    Counter counter;
    cout << counter.get() << endl;

    counter.inc();
    cout << counter.get() << endl;

    counter.reset();
    counter.inc2();
    cout << counter.get() << endl;

    return 0;
}
