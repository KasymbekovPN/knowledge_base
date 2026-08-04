#pragma once

#include <commands/command_history.hpp>
#include <commands/command_parser.hpp>
#include <task_domain/task_repository.hpp>

#include <istream>
#include <ostream>

namespace app {
    // Событийный цикл на входе приложения (аналог Day 1-2 темы плана: single-
    // threaded loop, читающий события из одного источника и гоняющий их по
    // очереди). Каждая строка ввода -> parseCommandLine -> либо ICommand,
    // дошедший до CommandHistory, либо служебное действие (undo/redo/quit).
    //
    // EventLoop НЕ ЗНАЕТ про View - вообще, ни единого include. Он мутирует
    // Model только через Command, и узнаёт результат не напрямую, а потому
    // что View сама подписана на события Model через EventBus. Точно так же
    // он не знает деталей ни одной команды - только про ICommand через
    // commands::parseCommandLine.
    class EventLoop {
    public:
        EventLoop(domain::TaskRepository&, commands::CommandHistory&, std::istream&, std::ostream&);

        // Крутится, пока не встретит "quit"/"exit" или EOF во входном потоке.
        void run();

    private:
        void handleLine(const std::string&);

        domain::TaskRepository& repo_;
        commands::CommandHistory& history_;
        std::istream& input_;
        std::ostream& output_;
        bool running_{true};
    };
}
