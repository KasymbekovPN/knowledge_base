#include "iplugin.hpp"

#include <algorithm>
#include <string>

class ReversePlugin: public IPlugin {
public:
    const char* name() const override { return "ReversePlugin"; }
    const char* execute(const char* input) override {
        result_ = input;
        std::ranges::reverse(result_);
        return result_.c_str();
    }
private:
    std::string result_;
};

extern "C" {
    int pluginApiVersion() { return PLUGIN_API_VERSION; }
    IPlugin* createPlugin() { return new ReversePlugin(); }
    void destroyPlugin(IPlugin* p) { delete p; }
}
