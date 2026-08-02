#pragma once

#include <string>

namespace commands {
    // Абстракция действия приложения. Command не знает про EventLoop, View
    // или EventBus напрямую - только про Model (через конкретные реализации),
    // над которой он выполняет операцию.
    class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;
        [[nodiscard]] virtual std::string description() const = 0;
    };
}

