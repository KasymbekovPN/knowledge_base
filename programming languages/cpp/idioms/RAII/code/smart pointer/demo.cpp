#include <iostream>
#include <memory>

void example_with_raii();
void example_without_raii();

int main() {
    example_with_raii();
    example_without_raii();

    return 0;
}

void example_with_raii() {
    std::unique_ptr<int> ptr = std::make_unique<int>(42);

    // Use ptr
    std::cout << *ptr << std::endl;

    // ... code, may be with exception ...
} // destructor calling -> delete is called automatically

void example_without_raii() {
    int *ptr = new int(42);

    // ... code, may be with exception ...

    delete ptr; // will not call in case of exception
}
