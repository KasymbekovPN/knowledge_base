#include <iostream>
#include "sum.h"

int main(int argc, char const *argv[]) {
    int first {5};
    int second {12};

    std::cout << "Sum of " << first << " & " << second << " => " << sum(first, second) << std::endl;
    
    return 0;
}
