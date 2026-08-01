#pragma once

#include <string>
#include <vector>

// Config - то, что раньше было захардкожено (заголовок приложения, подписи
// приоритетов) вынесено во внешний app.cfg. Ничего из этого не требует
// пересборки при изменении - см. data-driven demo с enemies.csv.
class Config {
public:
    static Config loadFromFile(const std::string& path);

    std::string priorityLabel(int priority);

    const std::string& appTitle() const { return appTitle_; }
    const std::vector<std::string>& priorityLabels() const { return priorityLabels_; }

    // priority хранится как int (0..N-1) в домене - Config отвечает только
    // за перевод его в человекочитаемую подпись для View.
    std::string priorityLabel(int priority) const;

private:
    std::string appTitle_{"Task board"};
    std::vector<std::string> priorityLabels_ = {"Low", "Medium", "High"};
};
