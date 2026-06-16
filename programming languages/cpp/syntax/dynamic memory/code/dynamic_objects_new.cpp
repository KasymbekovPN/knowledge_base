#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

void test_print(const string, int*);

int main(int argc, char const *argv[]) {
    const int BASE_VALUE = 42;
    int* ptr0 {new int{}};
    int* ptr1 {new int{BASE_VALUE}};
    int* ptr2 = new int{BASE_VALUE};
    int* ptr3 {new int(BASE_VALUE)};
    int* ptr4 = new int(BASE_VALUE);

    test_print("0 ptr0", ptr0);
    test_print("0 ptr1", ptr1);
    test_print("0 ptr2", ptr2);
    test_print("0 ptr3", ptr3);
    test_print("0 ptr4", ptr4);

    (*ptr0)++;
    (*ptr1)++;
    (*ptr2)++;
    (*ptr3)++;
    (*ptr4)++;

    test_print("1 ptr0", ptr0);
    test_print("1 ptr1", ptr1);
    test_print("1 ptr2", ptr2);
    test_print("1 ptr3", ptr3);
    test_print("1 ptr4", ptr4);

    return 0;
}

void test_print(const string key, int* p_value) {
    cout << "[TEST PRINT] " << key << " :: "  << *p_value << endl;
}
