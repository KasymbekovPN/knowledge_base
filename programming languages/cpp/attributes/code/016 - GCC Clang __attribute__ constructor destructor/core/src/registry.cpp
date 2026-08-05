#include "core/plugin.hpp"

#include <algorithm>

PluginRegistry& PluginRegistry::instance() {
    static PluginRegistry registry;
    return registry;
}

void PluginRegistry::registerPlugin(std::string name, PluginFactory factory) {
    plugins_.emplace_back(std::move(name), std::move(factory));
}

void PluginRegistry::unregisterPlugin(const std::string& name) {
    plugins_.erase(std::ranges::remove_if(
        plugins_, [&name](const auto& entry) { return entry.first == name; }).begin(),
        plugins_.end());
}

const std::vector<std::pair<std::string, PluginFactory>>& PluginRegistry::plugins() const {
    return plugins_;
}
