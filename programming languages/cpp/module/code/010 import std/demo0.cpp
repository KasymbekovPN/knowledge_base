/*
clang++ -std=c++23 -x c++-module --precompile "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\modules\std.ixx" -o std.pcm
if ($?) { clang++ -std=c++23 "-fmodule-file=std=std.pcm" demo0.cpp std.pcm -o app.exe }
*/

import std;

int main() {
    std::vector<int> v{3, 1, 2};
    std::sort(v.begin(), v.end());
    std::cout << std::format("v[{}] = {}", 0, v[0]);

    return 0;
}
