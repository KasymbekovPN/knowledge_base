#include <iostream>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    shared_ptr<int> null_ptr;
    cout << "null ptr <= " << null_ptr << endl << endl;

    shared_ptr<int> ptr0 {make_shared<int>(42)};
    shared_ptr<int> ptr1 {ptr0};

    cout << ptr0 << " <> " << *ptr0 << endl;
    cout << ptr1 << " <> " << *ptr1 << endl << endl;

    *ptr0 = 123;
    cout << ptr0 << " <> " << *ptr0 << endl;
    cout << ptr1 << " <> " << *ptr1 << endl << endl;

    const size_t SIZE{5};
    auto array {make_shared<int[]>(SIZE)};
    array[2] = 42;

    cout << "array <= ";
    for (size_t i{}; i < SIZE; i++){
        cout << array[i] << " ";
    }
    cout << endl;

    return 0;
}
