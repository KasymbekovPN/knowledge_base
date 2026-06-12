#include <iostream>
#include <type_traits>
#include <new>

struct V3 {
    float x, y, z;
    V3(float _x, float _y, float _z):
        x{_x},
        y{_y},
        z{_z} {}
};

using Storage = std::aligned_storage_t<sizeof(V3), alignof(V3)>;

int main() {
    alignas(Storage) char buffer[sizeof(Storage)];
    Storage* storage = reinterpret_cast<Storage*>(buffer);

    V3* vec = new(storage) V3(1.0f, 2.0f, 3.0f);

    std::cout
        << "{" << vec->x
        << ", " << vec->y
        << ", " << vec->z
        << "}" << std::endl;

    vec->~V3();

    return 0;
}
