#include "iplugin.hpp"

#include <string>
#include <format>

class HelloPlugin: public IPlugin {
public:
    const char* name() const override { return "HelloPlugin"; }
    const char* execute(const char* input) override {
        result_ = std::format("Hello, {}!", input);
        return result_.c_str();
    }
private:
    std::string result_;
};

// Точки входа - то, что host найдёт через dlsym по имени.
extern "C" {
    int pluginApiVersion() { return PLUGIN_API_VERSION; }
    IPlugin* createPlugin() { return new HelloPlugin(); }
    void destroyPlugin(IPlugin* p) { delete p; }
}
