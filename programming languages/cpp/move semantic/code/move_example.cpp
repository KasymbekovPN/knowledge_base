#include <iostream>

using std::cout;
using std::endl;

class CustomContainer {
private:
    int* data;
    size_t size;

public:
    CustomContainer(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "ctor" << endl;
    }

    ~CustomContainer() {
        delete[] data;
        cout << "dtor" << endl;
    }

    CustomContainer(CustomContainer&& other) noexcept:
        data(other.data),
        size(other.size) {
        other.data = nullptr;
        cout << "mv ctor" << endl;
    }

    CustomContainer& operator=(CustomContainer&& other) noexcept {
        cout << "mv operator=" << endl;
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
        }
        return *this;
    }

    CustomContainer(const CustomContainer& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "cpy ctor" << endl;
    }

    CustomContainer& operator=(const CustomContainer& other) {
        if (this != &other) {
            int* new_data = new int[other.size];
            std::copy(other.data, other.data + other.size, new_data);
            delete[] data;
            data = new_data;
            size = other.size;
            cout << "cpy operator=" << endl;
        }
        return *this;
    }
};

CustomContainer _create_tmp_container(const size_t size) {
    return CustomContainer(size);
}

int main() {
    const size_t SIZE = 1'000'000;

    cout << "### step 0" << endl;
    CustomContainer a(SIZE);

    cout << "### step 1" << endl;
    CustomContainer b = a;

    cout << "### step 2" << endl;
    CustomContainer c = std::move(a);

    cout << "### step 3" << endl;
    CustomContainer d = _create_tmp_container(SIZE);

    return 0;
}
