/*
clang++ -std=c++23 --precompile graphics_shapes.cppm -o graphics_shapes.pcm;
if ($?) { clang++ -std=c++23 --precompile graphics_gpu.cppm -o graphics_gpu.pcm }
if ($?) { clang++ -std=c++23 --precompile "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" graphics_textures.cppm -o graphics_textures.pcm }
if ($?) { clang++ -std=c++23 --precompile "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:textures=graphics_textures.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" graphics.cppm -o graphics.pcm }
if ($?) { clang++ -std=c++23 "-fmodule-file=graphics=graphics.pcm" "-fmodule-file=graphics:shapes=graphics_shapes.pcm" "-fmodule-file=graphics:textures=graphics_textures.pcm" "-fmodule-file=graphics:gpu=graphics_gpu.pcm" demo.cpp graphics_textures_impl.cpp graphics.pcm graphics_shapes.pcm graphics_textures.pcm graphics_gpu.pcm -o app.exe }
*/

import graphics;

#include <iostream>

void test_point() {
    std::cout << "### test_point\n";
    Point p{42, 12.34};
    p.print();
}

void test_rect() {
    std::cout << "### test_point\n";
    Rect rect{{142, 112.34}, {242, 212.34}};
    rect.print();
}

void test_texture() {
    std::cout << "### test_texture\n";
    Texture tex{{1, 2}, "some-path"};
    tex.print();
}

int main() {
    test_point();
    test_rect();
    test_texture();

    return 0;
}
