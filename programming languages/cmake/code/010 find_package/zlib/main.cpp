/*
cmake -B .build -DCMAKE_TOOLCHAIN_FILE=C:\projects\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build .build
*/

#include <iostream>
#include <format>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

int main() {
#ifdef HAVE_ZLIB
    std::string data{"Example data for compression: repeating data data data"};

    uLong src_len = data.size();
    uLong dst_len = compressBound(src_len);
    std::string compressed(dst_len, '\0');

    if (compress(reinterpret_cast<Bytef*>(&compressed[0]), &dst_len,
        reinterpret_cast<Bytef*>(data.data()), src_len) == Z_OK) {
        std::cout << std::format("Compressed: {} -> {} byte", src_len, dst_len);
    } else {
        std::cout << "Compression error";
    }
#else
    std::cout << "ZLIB is unreachable";
#endif
}
