#include <chrono>
#include <iostream>
#include <format>
#include <thread>

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

    // Держим процесс живым, чтобы успеть attach'иться gdb -p / lldb -p
    for (int i{}; i < 300q; ++i) {
        std::cout << std::format("tick {}\n", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

/*

Get-Process app | Select-Object Id
lldb -p 3420
(lldb) breakpoint set --file main.cpp --line 26
(lldb) continue
(lldb) print i

 */