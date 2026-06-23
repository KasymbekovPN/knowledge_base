module;

#include <vector>

export module types;

export using Byte = unsigned char;
export using Callback = void(*)(int);

export template<typename T>
using Vec = std::vector<T>; 
