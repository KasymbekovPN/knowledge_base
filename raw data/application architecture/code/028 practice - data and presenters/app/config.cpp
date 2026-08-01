#include "config.hpp"

#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
    std::vector<std::string> splitCsvList(const std::string& s) {
        std::istringstream ss{s};
        std::string item;
        std::vector<std::string> result;
        while (std::getline(ss, item, ',')) result.push_back(item);

        return result;
    }
}


Config Config::loadFromFile(const std::string& path) {
    Config cfg;
    std::ifstream file{path};
    if (!file) throw std::runtime_error{std::format("Cannot open file: {}", path)};

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        const auto EQ{line.find('=')};
        if (EQ == std::string::npos) continue;
        const std::string KEY{line.substr(0, EQ)};
        const std::string VALUE{line.substr(EQ + 1)};

        if (KEY == "app_title") cfg.appTitle_ = VALUE;
        else if (KEY == "priority_labels") cfg.priorityLabels_ = splitCsvList(VALUE);
        // неизвестные ключи молча игнорируются - конфиг может расти,
        // не ломая старые сборки движка (та же идея, что versioning интерфейсов)
    }

    return cfg;
}

std::string Config::priorityLabel(const int priority) {
    return priority < 0 || static_cast<size_t>(priority) >= priorityLabels_.size()
        ? "?"
        : priorityLabels_[priority];
}
