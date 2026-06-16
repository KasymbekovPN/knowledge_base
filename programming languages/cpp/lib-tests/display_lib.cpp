#include <iostream>

// clang++ -std=c++26 display_lib.cpp -o app.exe
// clang++ -std=c++26 -stdlib=libc++ display_lib.cpp -o app.exe

int main() {

#ifdef _LIBCPP_VERSION
    std::cout << "libc++ " << _LIBCPP_VERSION << '\n';
#elif defined(__GLIBCXX__)
    std::cout << "libstdc++ " << __GLIBCXX__ << '\n';   // дата-версия
#elif defined(_MSVC_STL_VERSION)
    std::cout << "MSVC STL " << _MSVC_STL_VERSION << '\n';
#else
    std::cout << "unknown stdlib\n";
#endif

    return 0;
}
