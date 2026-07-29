// Симулирует плагин, собранный против несовместимой (более старой/новой)
// версии API - например, кто-то забыл пересобрать плагин после того,
// как хост обновил интерфейс IPlugin. pluginApiVersion() намеренно
// возвращает не ту версию, чтобы показать, как host должен на это
// реагировать - НЕ падением, а явным отказом загрузки.

#include "iplugin.hpp"

#include <string>

class BadVersionPlugin : public IPlugin {
public:
    const char* name() const override {
        return "BadVersionPlugin";
    }
    const char* execute(const char* input) override {
        return "should never be called";
    }
};

extern "C" {
    // намеренно несовместимая версия
    int pluginApiVersion() { return PLUGIN_API_VERSION + 1; }
    IPlugin* createPlugin() { return new BadVersionPlugin(); }
    void destroyPlugin(IPlugin* p) { delete p; }
}
