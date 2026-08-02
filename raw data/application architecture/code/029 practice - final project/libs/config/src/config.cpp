#include "config/config.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <format>

namespace {
    std::vector<std::string> splitCsvList(const std::string& s) {
        std::istringstream ss(s);
        std::vector<std::string> result;
        std::string item;
        while (std::getline(ss, item, ',')) result.push_back(item);

        return result;
    }
}

Config Config::loadFromFile(const std::string& path) {
    Config cfg;
    std::ifstream file(path);
    if (!file) throw std::runtime_error(std::format("cannot open config-file: {}", path));

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        const auto EQ{line.find('=')};
        if (EQ == std::string::npos) continue;
        const std::string KEY{line.substr(0, EQ)};
        const std::string VALUE{line.substr(EQ + 1)};

        if (KEY == "app_title") {
            cfg.appTitle_ = VALUE;
        } else if (KEY == "priority_labels") {
            cfg.priorityLabels_ = splitCsvList(VALUE);
        }
    }

    return cfg;
}

std::string Config::priorityLabel(const int priority) const {
    return
        priority < 0 || static_cast<size_t>(priority) >= priorityLabels_.size()
        ? "?"
        : priorityLabels_[static_cast<size_t>(priority)];
}
