#include <iostream>

int main(int argc, char const *argv[]){
    wchar_t ch0 {L'A'};
    wchar_t ch1 {L'\x41'};

    std::wcout << "ch0 <= " << ch0 << "\n";
    std::wcout << "ch1 <= " << ch1 << "\n";

    return 0;
}
