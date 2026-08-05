#include "core/plugin.hpp"
#include "core/plugin_lifecycle.hpp"

#include <iostream>
#include <memory>

namespace {
    class PluginB : public IPlugin {
    public:
        [[nodiscard]] const char* name() const override { return "PluginB"; }
        void execute() const override {
            std::cout << "[PluginB] Greetings from plugin B!\n";
        }
    };
}

PLUGIN_LIFECYCLE(registerPluginB, unregisterPluginB)

static void registerPluginB() {
    PluginRegistry::instance().registerPlugin("PluginB", [] {
        return std::make_unique<PluginB>();
    });
    std::cout << "[PluginB] on-load: registered in PluginRegistry\n";
}

static void unregisterPluginB() {
    PluginRegistry::instance().unregisterPlugin("PluginB");
    std::cout << "[PluginB] on-unload: unregistered, plugin unloading\n";
}
