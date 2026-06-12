#include <string>
#include <memory>

class Wrapper {
private:
    std::string data;

public:
    Wrapper(const char* _str): data{_str} {}
    ~Wrapper() {}
};
