#pragma once

#include "commands/command.hpp"
#include <task_domain/task_repository.hpp>

#include <memory>
#include <string>

namespace commands {
    // Результат разбора одной строки ввода. EventLoop работает ТОЛЬКО через
    // эту структуру - он не парсит текст сам, не знает про синтаксис "add"/
    // "remove"/... и не знает деталей ни одной конкретной команды. Это и есть
    // узкий канал между "входом" (текст) и "моделью" (мутации через Command).
    enum class ParseAction { Dispatch, Undo, Redo, Help, Quit, Unknown };

    struct ParsedLine {
        ParseAction action{ParseAction::Unknown};
        // command - валиден только при action == Dispatch
        std::unique_ptr<ICommand> command{nullptr};
        // error - валиден при action == Unknown
        std::string error{};
    };

    ParsedLine parseCommandLine(const std::string&, domain::TaskRepository&);
}
