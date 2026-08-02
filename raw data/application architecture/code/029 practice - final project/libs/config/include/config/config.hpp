#pragma once

#include <string>
#include <vector>

// Config не знает ни про Model, ни про View, ни про EventBus - чисто
// данные + их парсинг. Любой модуль, которому нужны подписи/заголовок,
// зависит от config, а не наоборот.
class Config {
public:
    static Config loadFromFile(const std::string&);

    [[nodiscard]] const std::string& appTitle() const { return appTitle_; }
    [[nodiscard]] const std::vector<std::string>& priorityLabels() const { return priorityLabels_; }
    [[nodiscard]] std::string priorityLabel(int) const;
private:
    std::string appTitle_{"Task board"};
    std::vector<std::string> priorityLabels_ = {"Low", "Medium", "High"};
};
