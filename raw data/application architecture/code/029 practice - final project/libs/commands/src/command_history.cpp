#include "commands/command_history.hpp"

namespace commands {

void CommandHistory::execute(std::unique_ptr<ICommand> cmd) {
    cmd->execute();
    undoStack_.push_back(std::move(cmd));
    redoStack_.clear();
}

bool CommandHistory::undo() {
    //         if (undoStack_.empty()) return false;
    //         auto cmd = std::move(undoStack_.back());
    //         undoStack_.pop_back();
    //         cmd->undo();
    //         redoStack_.push_back(std::move(cmd));
    //         return true;
}

bool CommandHistory::redo() {
    //         if (redoStack_.empty()) return false;
    //         auto cmd = std::move(redoStack_.back());
    //         redoStack_.pop_back();
    //         cmd->execute();
    //         undoStack_.push_back(std::move(cmd));
    //         return true;
}

}
