#include <iostream>
#include <cstdlib>
#include <iostream>
#include <format>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
    // --- Аргументы командной строки ---
    std::cout << "argc = " << argc << "\n";
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";   // <-- breakpoint здесь
    }

    // --- Переменные окружения ---
    const char* log_level = std::getenv("LOG_LEVEL");
    const char* asio_disable_threads = std::getenv("BOOST_ASIO_DISABLE_THREADS");

    std::string log_level_str = log_level ? log_level : "(NOT SET)";
    std::string threads_str = asio_disable_threads ? asio_disable_threads : "(NOT SET)";

    std::cout << "LOG_LEVEL = " << log_level_str << "\n";
    std::cout << "BOOST_ASIO_DISABLE_THREADS = " << threads_str << "\n";  // <-- и здесь

    // Держим процесс живым, чтобы успеть attach'иться gdb -p / lldb -p
    for (int i = 0; i < 300; ++i) {
        std::cout << "tick " << i << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));  // <-- breakpoint после attach сюда
    }

    return 0;
}

/*

###
Get-Process app | Select-Object Id
lldb -p 12345
command script import lldb_automation.py
dump-threads id
breakpoint set --file main.cpp --line 28
breakpoint command add 1 -F lldb_automation.every_nth_callback

###
Get-Process app | Select-Object Id
gdb -p 12345
(gdb) dump-threads id
(gdb) auto-log-loop main.cpp:26 i 10
(gdb) python EveryNthBreakpoint('main.cpp:28', 5)

 */
