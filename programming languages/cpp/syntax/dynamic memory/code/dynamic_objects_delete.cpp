#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

int* create_ptr(const int);
int* delete_ptr(int*);
void print(string, int*);

int main(int argc, char const *argv[]) {
    const int FIRST_VALUE {42};
    const int SECOND_VALUE {43};
    
    int* ptr = create_ptr(FIRST_VALUE);
    print("after first creation", ptr);
    ptr = delete_ptr(ptr);
    print("after first deleting", ptr);

    ptr = create_ptr(SECOND_VALUE);
    print("after second creation", ptr);
    
    ptr = delete_ptr(ptr);
    print("after second deleting", ptr);

    return 0;
}

int* create_ptr(const int _init_value) {
    int* ptr {new int{_init_value}};
    return ptr;
}

int* delete_ptr(int* _ptr) {
    delete _ptr;
    _ptr = nullptr;
    return _ptr;
}

void print(string _label, int* _ptr) {
    cout << "[PRINT] " << _label << " <= " << _ptr;
    if (_ptr != nullptr) {
       cout  << " <= " << *_ptr;
    }
    cout << endl;
}
