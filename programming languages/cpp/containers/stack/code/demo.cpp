#include <iostream>
#include <stack>

void _print_stack(std::stack<int>&);

int main(int argc, char const *argv[]) {
    std::stack<int> st;
    st.push(1);
    st.push(2);
    st.push(3);
    _print_stack(st);

    while (!st.empty()) {
        std::cout << st.top() << " ";
        st.pop();
    }
    std::cout << std::endl;
    
    _print_stack(st);

    return 0;
}

void _print_stack(std::stack<int>& st) {
    std::cout
        << "Size: " << st.size()
        << ", is empty" << std::boolalpha << ": " << st.empty()
        << std::noboolalpha
        << std::endl;
}
