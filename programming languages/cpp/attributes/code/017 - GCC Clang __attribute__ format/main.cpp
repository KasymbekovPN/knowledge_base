#include <cstdio>
#include <cstdarg>

namespace {
    // archetype: printf
    // string-index: 1 (формат — первый аргумент, если это функция-член, счёт с учётом this — со второго)
    // first-to-check: 2 (variadic-аргументы начинаются со второго параметра)
    __attribute__((format(printf, 1, 2)))
    void logMessage(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    class Logger {
    public:
        __attribute__((format(printf, 2, 3)))
        void log(const char* fmt, ...) const {
            va_list args;
            va_start(args, fmt);
            vprintf(fmt, args);
            va_end(args);
        }
    };
}

int main() {
    logMessage("value: %d\n", 42);
    logMessage("value: %d\n", "hello");
    logMessage("value: %d\n");

    Logger logger;
    logger.log("value: %d\n", 43);
}
