#include <iostream>
#include <format>
#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
    // --- Аргументы командной строки ---
    std::cout << std::format("argc = {}\n", argc);
    for (int i{}; i < argc; ++i) {
        std::cout << std::format("argv[{}] = {}\n", i, argv[i]);
    }

    // --- Переменные окружения ---
    const char* LOG_LEVEL = std::getenv("LOG_LEVEL");
    const char* ASIO_DOSABLE_THREADS = std::getenv("BOOST_ASIO_DISABLE_THREADS");

    const std::string slog_level = LOG_LEVEL ? LOG_LEVEL : "NOT SET";
    const std::string sthreads = ASIO_DOSABLE_THREADS ? ASIO_DOSABLE_THREADS : "NOT SET";

    std::cout << std::format("LOG_LEVEL: {}\n", slog_level);
    std::cout << std::format("BOOST_ASIO_DISABLE_THREADS: {}\n", sthreads);

    return 0;
}

/*

cmake --preset release
cmake --build --preset release
cmake --build --preset release --config=Release

###

C:\msys64\clang64\bin\gdb.exe --args .\build\debug\app.exe one two --flag=42
set environment LOG_LEVEL=debug
set environment BOOST_ASIO_DISABLE_THREADS=1
break main.cpp:21
run
print argv[1]
print argv[2]
print argv[3]
print slog_level
print s_threads

###

lldb -- .\build\debug\app.exe one two --flag=42
settings set target.env-vars LOG_LEVEL=debug BOOST_ASIO_DISABLE_THREADS=1
breakpoint set --file main.cpp --line 21
run
print argv[1]
print argv[2]
print argv[3]
print slog_level
print sthreads

 */