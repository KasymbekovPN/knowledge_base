#include "core/plugin.hpp"
#include "core/plugin_lifecycle.hpp"

#include <iostream>
#include <memory>

namespace {
    class PluginA: public IPlugin {
    public:
        [[nodiscard]] const char* name() const override { return "PluginA"; }
        void execute() const override {
            std::cout << "[PluginA] Hello from plugin A!\n";
        }
    };
}

// На GCC/Clang это разворачивается в пару __attribute__((constructor))/
// ((destructor)); на MSVC — в DllMain. См. core/plugin_lifecycle.h.
PLUGIN_LIFECYCLE(registerPluginA, unregisterPluginA)

static void registerPluginA() {
    PluginRegistry::instance().registerPlugin("PluginA", []() {
        return std::make_unique<PluginA>();
    });
    std::cout << "[PluginA] on-load: registered in PluginRegistry\n";
}

// КРИТИЧНО вызвать unregisterPlugin здесь, до фактической выгрузки
// библиотеки — запись в реестре хранит std::function с кодом, живущим
// в этой .so/.dll (см. README, раздел про найденный при тестировании баг).
static void unregisterPluginA() {
    PluginRegistry::instance().unregisterPlugin("PluginA");
    std::cout << "[PluginA] on-unload: unregistered, plugin unloading\n";
}
