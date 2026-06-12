#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>
#include <string>
#include <mutex>
#include <chrono>

using namespace std;

struct Wrapper {
    shared_mutex mtx;
    vector<int> data;
};

void print_vector(vector<int>&, const string&&);
void read(Wrapper&, const int);
void write(Wrapper&, const int);

int main(int argc, char const *argv[]) {
    constexpr int QUANTITY {3};
    Wrapper wrapper;
    thread tread ([&]() {
        for (size_t i = 0; i < QUANTITY; i++) {
            this_thread::sleep_for(chrono::milliseconds(10));
            read(wrapper, i);
        }
    });

    thread twrite ([&]() {
        for (size_t i = 0; i < QUANTITY; i++) {
            this_thread::sleep_for(chrono::milliseconds(10));
            write(wrapper, rand() % 100);
        }
    });

    tread.join();
    twrite.join();

    return 0;
}

void print_vector(vector<int>& _container, const string&& _lbl) {
    string delimiter {""};
    cout << "[" << _lbl << "] {";
    for (auto &item: _container) {
        cout << delimiter << item;
        delimiter = ",";
    }
    cout << "}" << endl;
}

void read(Wrapper& _wrapper, const int _index) {
    shared_lock<shared_mutex> lock(_wrapper.mtx);
    print_vector(_wrapper.data, to_string(_index));
}

void write(Wrapper& _wrapper,  int _value) {
    lock_guard<shared_mutex>  lock(_wrapper.mtx);
    _wrapper.data.push_back(_value);
}
