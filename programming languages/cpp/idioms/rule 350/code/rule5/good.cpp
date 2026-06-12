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

    GoodImpl(const GoodImpl& _other) {
        std::cout << "copy ctor" << std::endl;
        copy_from(_other.data);
    }

    GoodImpl(GoodImpl&& _other) noexcept {
        std::cout << "move ctor" << std::endl;
        data = _other.data;
        _other.data = nullptr;
    }

    GoodImpl& operator=(const GoodImpl& _other) {
        std::cout << "assign op" << std::endl;
        if (this != &_other) {
            delete[] data;
            copy_from(_other.data);
        }
        return *this;
    }

    GoodImpl& operator=(GoodImpl&& _other) noexcept {
        std::cout << "mv assign op" << std::endl;
        if (this != &_other) {
            delete[] data;
            data = _other.data;
            _other.data = nullptr;
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
    auto c{std::move(a)};
}
