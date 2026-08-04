#include "commands/command_parser.hpp"
#include "commands/task_commands.hpp"

#include <sstream>
#include <format>

namespace commands {

namespace {

// trim + разбить строку на первое слово (команду) и остаток (аргументы)
std::pair<std::string, std::string> splitHead(const std::string& line) {
    const auto START{line.find_first_not_of(" \t")};
    if (START == std::string::npos) return {"", ""};
    const auto SPACE_POS{line.find_first_of(" \t", START)};
    if (SPACE_POS == std::string::npos) return {line.substr(START), ""};
    const std::string HEAD{line.substr(START, SPACE_POS - START)};
    const auto REST_START{line.find_first_not_of(" \t", SPACE_POS)};
    const std::string REST{REST_START == std::string::npos ? "" : line.substr(REST_START)};

    return {HEAD, REST};
}

struct HandlerContext {
    std::string& head;
    std::string& rest;
    domain::TaskRepository& repo;
};

std::unordered_map<std::string, std::function<ParsedLine(const HandlerContext&)>> LINE_HANDLERS{
    {
        "",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Unknown, .command = nullptr, .error = ""};
        }
    },
    {
        "quit",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Quit, .command = nullptr, .error = ""};
        }
    },
    {
        "exit",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Quit, .command = nullptr, .error = ""};
        }
    },
    {
        "undo",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Undo, .command = nullptr, .error = ""};
        }
    },
    {
        "redo",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Redo, .command = nullptr, .error = ""};
        }
    },
    {
        "help",
        [](const HandlerContext& context) {
            return ParsedLine{.action = ParseAction::Help, .command = nullptr, .error = ""};
        }
    },
    {
        "add",
        [](const HandlerContext& context) {
            std::istringstream ss{context.rest};
            int priority{};
            if (!(ss >> priority))
                return ParsedLine{
                .action = ParseAction::Unknown,
                .command = nullptr,
                .error = "add wait 'add <priority> <title>'"
            };

            std::string title{};
            std::getline(ss, title);
            const auto TITLE_START{title.find_first_not_of(' ')};
            title = (TITLE_START == std::string::npos) ? "" : title.substr(TITLE_START);
            if (title.empty()) return ParsedLine{ParseAction::Unknown, nullptr, "add: empty title"};

            return ParsedLine{
                .action = ParseAction::Dispatch,
                .command = makeAddTaskCommand(context.repo, title, priority),
                .error = ""
            };
        }
    },
    {
        "remove",
        [](const HandlerContext& context) {
            std::istringstream ss{context.rest};
            int id{0};
            if (!(ss >> id)) return ParsedLine{
                .action = ParseAction::Unknown,
                .command = nullptr,
                .error = std::format("{} wait '{} <id>'", context.head, context.head)
            };

            return ParsedLine{
                .action = ParseAction::Dispatch,
                .command = makeRemoveTaskCommand(context.repo, id),
                .error = ""
            };
        }
    },
    {
        "complete",
        [](const HandlerContext& context) {
            std::istringstream ss{context.rest};
            int id{0};
            if (!(ss >> id)) return ParsedLine{
                .action = ParseAction::Unknown,
                .command = nullptr,
                .error = std::format("{} wait '{} <id>'", context.head, context.head)
            };

            return ParsedLine{
                .action = ParseAction::Dispatch,
                .command = makeCompleteTaskCommand(context.repo, id),
                .error = ""
            };
        }
    },
    {
        "reopen",
        [](const HandlerContext& context) {
            std::istringstream ss{context.rest};
            int id{0};
            if (!(ss >> id)) return ParsedLine{
                .action = ParseAction::Unknown,
                .command = nullptr,
                .error = std::format("{} wait '{} <id>'", context.head, context.head)
            };

            return ParsedLine{
                .action = ParseAction::Dispatch,
                .command = makeReopenTaskCommand(context.repo, id),
                .error = ""
            };
        }
    }
};

const auto DEFAULT_HANDLER = [](const HandlerContext& context) {
    return ParsedLine{
        .action = ParseAction::Unknown,
        .command = nullptr,
        .error = std::format("unknown command: {} (to type 'help')", context.head)
    };
};

}

ParsedLine parseCommandLine(const std::string& line, domain::TaskRepository& repo) {
    auto [head, rest] = splitHead(line);

    const HandlerContext context{.head = head, .rest = rest, .repo = repo};
    if (LINE_HANDLERS.contains(head)) {
        return LINE_HANDLERS.at(head)(context);
    }

    return DEFAULT_HANDLER(context);
}

}
