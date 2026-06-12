#include <iostream>
#include <cstring>

class GoodImpl {
private:
    char* data;

    void copy_from(const char* _str) {
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

public:
    GoodImpl(const char* _str) {
        std::cout << "ctor" << std::endl;
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

    GoodImpl(const GoodImpl& other) {
        std::cout << "copy ctor" << std::endl;
        copy_from(other.data);
    }

    GoodImpl& operator=(const GoodImpl& other) {
        std::cout << "assign op" << std::endl;
        if (this != &other) {
            delete[] data;
            copy_from(other.data);
        }
        return *this;
    }

    ~GoodImpl() {
        std::cout << "dtor" << std::endl;
        delete[] data;
    }
};

void test();

int main() {
    test();

    return 0;
}

void test() {
    GoodImpl a{"Hello"};
    auto b = a;
}
