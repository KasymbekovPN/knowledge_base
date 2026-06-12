#include <iostream>
#include <mutex>
#include <thread>
#include <stack>
#include <optional>
#include <condition_variable>

class Stack {

public:
    void push(int _value) {
        {
            std::lock_guard lock{mtx};
            data.push(_value);
        }
        cv.notify_one();
    }

    int pop() {
        std::unique_lock lock{mtx};
        cv.wait(lock, [this]() {
            return !data.empty();
        });
        int result = data.top();
        data.pop();

        return result;
    }

private:
    std::mutex mtx;
    std::stack<int> data;
    std::condition_variable cv;
};

int main() {
    Stack st;

    std::thread t0{[&]() {
        std::cout << "T0: " << st.pop() << "\n";
    }};

    std::thread t1{[&]() {
        st.push(42);
    }};

    t0.join();
    t1.join();

    return 0;
}
