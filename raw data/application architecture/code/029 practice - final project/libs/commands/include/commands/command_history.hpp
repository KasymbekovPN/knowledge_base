#pragma once

#include "commands/command.hpp"

#include <memory>
#include <vector>

namespace commands {
    // Диспетчер команд с undo/redo. Не знает НИЧЕГО про конкретные команды -
    // работает только через ICommand. execute() чистит redo-стек, как и
    // положено (после нового действия старая "будущая" история недействительна).
    class CommandHistory {
    public:
        void execute(std::unique_ptr<ICommand>);
        bool undo();
        bool redo();
        [[nodiscard]] bool canUndo() const { return !undoStack_.empty(); }
        [[nodiscard]] bool canRedo() const { return !redoStack_.empty(); }

    private:
        std::vector<std::shared_ptr<ICommand>> undoStack_;
        std::vector<std::shared_ptr<ICommand>> redoStack_;
    };

}
