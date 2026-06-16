#include <iostream>
#include <utility>

using std::cout;
using std::endl;

class CustomContainer {
private:
    size_t size;
    int* data;

public:
    CustomContainer(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "CC ctor" << endl;
    }

    CustomContainer(const CustomContainer& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "CC copy ctor" << endl;
    }

    ~CustomContainer() {
        if (data) {
            delete[] data;
        }
        cout << "CC dctor" << endl;
    }

    void print() const {
        cout << "CC print" << endl;
    }
};

class CustomContainerM {
private:
    size_t size;
    int* data;

public:
    CustomContainerM(const size_t size):
        size(size),
        data(new int[size]) {
        cout << "CCM ctor" << endl;
    }

    CustomContainerM(const CustomContainerM& other):
        size(other.size),
        data(new int[other.size]) {
        std::copy(other.data, other.data + size, data);
        cout << "CCM copy ctor" << endl;
    }

    CustomContainerM(CustomContainerM&& other) noexcept:
        size(other.size) {
        data = other.data;
        other.data = nullptr;
        cout << "CCM move ctor" << endl;
    }

    CustomContainerM& operator=(const CustomContainerM& other){
        if (this != &other) {
            size = other.size;
            int* new_data = new int[size];
            std::copy(other.data, other.data + other.size, new_data);
            delete[] data;
            data = new_data;
            cout << "CCM copy =" << endl;
        }
        return *this;
    }
    
    CustomContainerM& operator=(CustomContainerM&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    ~CustomContainerM() {
        if (data) {
            delete[] data;
        }
        cout << "CCM dctor" << endl;
    }

    void print() const {
        cout << "CCM print" << endl;
    }
};

CustomContainer create_cc_rvo() {
    cout << "create CC RVO" << endl;
    return CustomContainer(100);
}

CustomContainer create_cc_nrvo() {
    cout << "create CC NRVO" << endl;
    CustomContainer obj = CustomContainer(100);
    return obj;
}

CustomContainer create_cc_move() {
    cout << "create CC MOVE" << endl;
    return std::move(CustomContainer(100));
}

CustomContainerM create_ccm_rvo() {
    cout << "create CC RVO" << endl;
    return CustomContainerM(100);
}

CustomContainerM create_ccm_nrvo() {
    cout << "create CC NRVO" << endl;
    CustomContainerM obj = CustomContainerM(100);
    return obj;
}

CustomContainerM create_ccm_move() {
    cout << "create CC MOVE" << endl;
    return std::move(CustomContainerM(100));
}

void recieve_cc(CustomContainer value) {
    cout << "print_cc as value" << endl;
    value.print();
}

void recieve_cc(CustomContainer& value) {
    cout << "print_cc as ref" << endl;
    value.print();
}

void recieve_cc(CustomContainer&& value) {
    cout << "print_cc as rref" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM value) {
    cout << "print_cc as value" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM& value) {
    cout << "print_cc as ref" << endl;
    value.print();
}

void recieve_ccm(CustomContainerM&& value) {
    cout << "print_cc as rref" << endl;
    value.print();
}

int main() {
    CustomContainerM cc = CustomContainerM(100);
    return 0;
}
