#include <iostream>
#include <format>
#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <map>
#include <string>

struct Task {
    std::string name;
    int priority;
};

int main() {
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    std::vector<std::string> names = {"Pablo", "Anna", "Max"};
    std::map<std::string, int> scores = {
        {"alice", 95},
        {"bob", 87},
        {"carol", 92}};
    std::unique_ptr<Task> task = std::make_unique<Task>(
        Task{"Deploy service", 1});
    std::shared_ptr<int> shared_counter = std::make_shared<int>(42);
    std::optional<int> maybe_value = 7;
    std::optional<int> empty_value = std::nullopt;
    std::variant<int, std::string> var_value = "hello variant";
    std::vector<Task> tasks = {
        {"Build", 2},
        {"Test", 1},
        {"Deploy", 3}};

    std::cout << "numbers.size() = " << numbers.size() << "\n";  // <-- breakpoint здесь
    std::cout << "scores[\"bob\"] = " << scores["bob"] << "\n";
    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 33
command script import lldb_task_formatter.py
type summary add Task -F lldb_task_formatter.task_summary
run

###
C:\msys64\clang64\bin\gdb.exe -q -x gdb_task_formatter.py .\build\debug\app.exe

C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
source gdb_task_formatter.py
break main.cpp:33
run


*/
