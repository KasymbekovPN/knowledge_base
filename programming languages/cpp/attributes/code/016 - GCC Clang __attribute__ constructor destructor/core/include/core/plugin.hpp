#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32) || defined(__CIGWIN__)
    #if defined(CORE_EXPORTS)
        #define CORE_API __declspec(dllexport)
    #else
        #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API __attribute__((visibility("default")))
#endif

// Интерфейс, который должен реализовать каждый плагин.
class IPlugin {
public:
    virtual ~IPlugin() = default;
    [[nodiscard]] virtual const char* name() const = 0;
    virtual void execute() const = 0;
};

using PluginFactory = std::function<std::unique_ptr<IPlugin>()>;

// Глобальный реестр плагинов. Живёт внутри libcore.so — единственный экземпляр
// на процесс, потому что и host, и каждая plugin_*.so линкуются с одной и той же
// core-библиотекой, а значит динамический линкер резолвит обращения к
// PluginRegistry::instance() в один и тот же объект в памяти.
class CORE_API PluginRegistry {
public:
    static PluginRegistry& instance();

    void registerPlugin(std::string, PluginFactory);

    // ВАЖНО: перед dlclose() плагин обязан вызвать это для себя из своего
    // __attribute__((destructor)). PluginFactory — это std::function,
    // хранящий указатель на код (лямбду), физически находящийся внутри
    // .so самого плагина. Если оставить запись в реестре висеть после
    // dlclose(), то при разрушении этой std::function (например, при
    // выходе из процесса) программа попытается выполнить код по адресу,
    // который уже был отмаплен из памяти -> segfault.
    void unregisterPlugin(const std::string&);

    [[nodiscard]] const std::vector<std::pair<std::string, PluginFactory>>& plugins() const;

private:
    std::vector<std::pair<std::string, PluginFactory>> plugins_;
};
