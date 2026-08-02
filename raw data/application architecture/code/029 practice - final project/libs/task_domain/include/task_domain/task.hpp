#pragma once

#include <string>

namespace domain {
    struct Task {
        int id{};
        std::string title{};
        // 0..N-1, подпись переводится через Config, не хардкодится тут
        int priority{};
        bool done{false};
    };

    // События Model. Commands/View подписываются на них, сама Model ничего
    // не знает ни про Command, ни про View - это чисто её "исходящий контракт".
    struct TaskAdded { int id{}; std::string title{}; int priority{}; };
    // несём снимок удалённой задачи - нужен для undo и для View
    struct TaskRemoved { int id{}; Task removed{}; };
    struct TaskCompleted { int id{}; };
    struct TaskReopened { int id{}; };
    struct TaskRestored { int id{}; };
}
