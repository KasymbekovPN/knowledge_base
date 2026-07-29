// "TaskApp" (простой список задач),
// собранный из всего, что разбирали в этом блоке:
//   - App - composition root (ownership tree, RAII)
//   - EventBus - компоненты общаются, не зная друг о друге
//   - Command - 3 действия (Add/Complete/Remove) с undo/redo
//
// Всё в одном файле для наглядности, но границы между кусками - это ровно
// границы будущих модулей (в следующем блоке плана - как их фактически
// разнести по отдельным библиотекам/файлам).

#include <algorithm>
#include <functional>
#include <iostream>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <stack>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

// =====================================================================
// EventBus (тот же механизм, что разбирали отдельно): типобезопасная
// шина, RAII-подписка через Connection.
// =====================================================================
class EventBus {
public:
    using SubscriptionId = std::size_t;

    class Connection {
    public:
        Connection() = default;
        Connection(Connection&& other) noexcept { *this = std::move(other); }
        Connection& operator=(Connection&& other) noexcept {
            if (this != &other) {
                disconnect();
                bus_ = other.bus_;
                type_ = other.type_;
                id_ = other.id_;
                other.bus_ = nullptr;
            }
            return *this;
        }
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;
        ~Connection() { disconnect(); }

        void disconnect() {
            if (!bus_) return;
            bus_->unsubscribeRaw(type_, id_);
            bus_ = nullptr;
        }
    private:
        friend class EventBus;
        Connection(EventBus* bus, const std::type_index type, const SubscriptionId id):
            bus_(bus), type_(type), id_(id) {}

        EventBus* bus_{nullptr};
        std::type_index type_{typeid(void)};
        SubscriptionId id_{0};
    };

    template <typename EventT>
    Connection subscribe(std::function<void(const EventT&)> handler) {
        const auto type{std::type_index(typeid(EventT))};
        const SubscriptionId id{nextId_++};
        handlers_[type].push_back({
            id,
            [handler](const void* e) { handler(*static_cast<const EventT*>(e)); }
        });

        return Connection{this, type, id};
    }

    template <typename EventT>
    void publish(const EventT& event) {
        const auto it{handlers_.find(std::type_index(typeid(EventT)))};
        if (it == handlers_.end()) return;

        for (const auto list_copy = it->second;
            const auto& fn: list_copy | std::views::values) {
            fn(&event);
        }
    }

private:
    void unsubscribeRaw(const std::type_index type, const SubscriptionId id) {
        const auto it{handlers_.find(type)};
        if (it == handlers_.end()) return;
        auto& list = it->second;
        list.erase(
            std::ranges::remove_if(list, [id](const auto& entry){ return entry.first == id; }).begin(),
            list.end());
    }

    std::unordered_map<
        std::type_index,
        std::vector<std::pair<SubscriptionId, std::function<void(const void*)>>>
    > handlers_;
    SubscriptionId nextId_{1};
};

// =====================================================================
// События приложения.
// =====================================================================
struct TaskAdded {int id; std::string title; };
struct TaskRemoved {int id; std::string title; };
struct TaskCompleted { int id; };
struct TaskReopened { int id; };

// =====================================================================
// Domain: Task + TaskRepository. Ничего не знает ни про Command, ни про
// EventBus, ни про undo/redo - обычный класс с прямыми операциями
// (Receiver в терминах Command pattern).
// =====================================================================
struct Task {
    int id{0};
    std::string title;
    bool done{false};
};

class TaskRepository {
public:
    int addTask(const std::string& title) {
        int id{nextId_++};
        tasks_.emplace_back(id, title, false);
        return id;
    }

    void insertTaskAt(const size_t index, const Task& task) {
        const auto pos{
            tasks_.begin() +
            static_cast<std::ptrdiff_t>(std::min(index, tasks_.size())) };
        tasks_.insert(pos, task);
    }

    std::pair<Task, size_t> removeTask(const int id) {
        const auto it{std::ranges::find_if(
            tasks_,
            [id](const Task& t){ return t.id == id; })};
        if (it == tasks_.end()) throw std::runtime_error(std::format("task not found {}", id));
        const size_t index{static_cast<size_t>(std::distance(tasks_.begin(), it))};
        Task copy = *it;
        tasks_.erase(it);

        return {copy, index};
    }

    void removeTaskById(const int id) {
        const auto it{std::ranges::find_if(
            tasks_,
            [id](const Task& t){ return t.id == id; })};
        if (it != tasks_.end()) tasks_.erase(it);
    }

    void setDone(const int id, const bool done) {
        const auto it{std::ranges::find_if(
            tasks_,
            [id](const Task& t){ return t.id == id; })};
        if (it != tasks_.end()) it->done = done;
    }

    const std::vector<Task>& all() const { return tasks_; }

private:
    std::vector<Task> tasks_;
    int nextId_{1};
};

// =====================================================================
// Command: контракт + 3 конкретных действия приложения.
// =====================================================================
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

class AddTaskCommand: public ICommand {
public:
    AddTaskCommand(TaskRepository& repo, EventBus& bus, std::string title):
        repo_(repo), bus_(bus), title_(std::move(title)) {}

    void execute() override {
        if (!id_.has_value()) {
            // первый execute - реально создаём задачу, получаем новый id
            id_ = repo_.addTask(title_);
        } else {
            // повторный execute - это redo после undo; id уже известен,
            // просто вставляем ту же задачу обратно в конец
            repo_.insertTaskAt(repo_.all().size(), Task{*id_, title_, false});
        }
        bus_.publish(TaskAdded{*id_, title_});
    }

    void undo() override {
        bus_.publish(TaskRemoved{*id_, title_});
        repo_.removeTaskById(*id_);
    }

    std::string description() const override {
        return std::format("AddTask({})", title_);
    }

    int id() const { return id_.value(); }
private:
    TaskRepository& repo_;
    EventBus& bus_;
    std::string title_;
    std::optional<int> id_;
};

class CompleteTaskCommand: public ICommand {
public:
    CompleteTaskCommand(TaskRepository& repo, EventBus& bus, const int id):
        repo_(repo), bus_(bus), id_(id) {}

    void execute() override {
        repo_.setDone(id_, true);
        bus_.publish(TaskCompleted{id_});
    }

    void undo() override {
        repo_.setDone(id_, false);
        bus_.publish(TaskReopened{id_});
    }

    std::string description() const override {
        return std::format("CompleteTask(#{})", id_);
    }
private:
    TaskRepository& repo_;
    EventBus& bus_;
    int id_{0};
};

class RemoveTaskCommand: public ICommand {
public:
    RemoveTaskCommand(TaskRepository& repo, EventBus& bus, const int id):
        repo_(repo), bus_(bus), id_(id) {}

    void execute() override {
        const auto& [task, index] = repo_.removeTask(id_);
        removedTask_ = task;
        removedIndex_ = index;
        bus_.publish(TaskRemoved{id_, task.title});
    }

    void undo() override {
        repo_.insertTaskAt(removedIndex_, removedTask_);
        bus_.publish(TaskAdded{removedTask_.id, removedTask_.title});
    }

    std::string description() const override {
        return std::format("RemoveTask(#{})", id_);
    }

private:
    TaskRepository& repo_;
    EventBus& bus_;
    int id_{0};
    Task removedTask_{};
    size_t removedIndex_{0};
};

// =====================================================================
// Invoker - undo/redo стек, ничего не знает про Task/TaskRepository.
// =====================================================================
class CommandManager {
public:
    void execute(std::unique_ptr<ICommand> cmd) {
        cmd->execute();
        undoStack_.push(std::move(cmd));
        while(!redoStack_.empty()) redoStack_.pop();
    }

    bool undo () {
        if (undoStack_.empty()) return false;
        auto cmd{std::move(undoStack_.top())};
        undoStack_.pop();
        cmd->undo();
        redoStack_.push(std::move(cmd));

        return true;
    }

    bool redo () {
        if (redoStack_.empty()) return false;
        auto cmd{std::move(redoStack_.top())};
        redoStack_.pop();
        cmd->execute();
        undoStack_.push(std::move(cmd));

        return true;
    }

private:
    std::stack<std::unique_ptr<ICommand>> undoStack_;
    std::stack<std::unique_ptr<ICommand>> redoStack_;
};

// =====================================================================
// Подписчики на события - не знают друг о друге, только про EventBus.
// =====================================================================
class ActivityLog {
public:
    explicit ActivityLog(EventBus& bus) {
        addedConn_ = bus.subscribe<TaskAdded>([this](const TaskAdded& e) {
            std::cout << std::format("  [activity] task #{} added '{}'\n", e.id, e.title);
        });
        removedConn_ = bus.subscribe<TaskRemoved>([this](const TaskRemoved& e) {
            std::cout << std::format("  [activity] task #{} removed '{}'\n", e.id, e.title);
        });
        completedConn_ = bus.subscribe<TaskCompleted>([this](const TaskCompleted& e) {
            std::cout << std::format("  [activity] task #{} done\n", e.id);
        });
        reopenedConn_ = bus.subscribe<TaskReopened>([this](const TaskReopened& e) {
            std::cout << std::format("  [activity] task #{} reopened\n", e.id);
        });
    };

private:
    EventBus::Connection addedConn_, removedConn_, completedConn_, reopenedConn_;
};

class CompletionStats {
public:
    explicit CompletionStats(EventBus& bus) {
        completedConn_ = bus.subscribe<TaskCompleted>([this](const TaskCompleted& e) { ++completed_; });
        reopenedConn_ = bus.subscribe<TaskReopened>([this](const TaskReopened& e) { --completed_; });
    }
    int completed() const { return completed_; }
private:
    int completed_{0};
    EventBus::Connection completedConn_, reopenedConn_;
};

// =====================================================================
// App - composition root. Единственное место, где всё собирается вместе.
// Владеет всем по значению (дерево владения), наружу отдаёт узкий API
// (addTask/completeTask/removeTask/undo/redo), а не доступ к внутренностям.
// =====================================================================
class App {
public:
    App(): activityLog_{bus_}, stats_{bus_} {}

    int addTask(const std::string& title) {
        auto cmd{std::make_unique<AddTaskCommand>(repo_, bus_, title)};
        const AddTaskCommand* raw{cmd.get()};
        commands_.execute(std::move(cmd));

        return raw->id();
    }

    void completeTask(const int id) {
        commands_.execute(std::make_unique<CompleteTaskCommand>(repo_, bus_, id));
    }

    void removeTask(const int id) {
        commands_.execute(std::make_unique<RemoveTaskCommand>(repo_, bus_, id));
    }

    void undo() { commands_.undo(); }
    void redo() { commands_.redo(); }

    void printState() const {
        std::cout << "  state: ";
        for (const auto&[id, title, done]: repo_.all()) {
            std::cout << std::format("[#{} {} {}]", id, title, done ? "done" : "");
        }
        std::cout << '\n';
    }

    int completedCount() const { return stats_.completed(); }

private:
    EventBus bus_;
    TaskRepository repo_;
    CommandManager commands_;
    ActivityLog activityLog_;
    CompletionStats stats_;
};

int main() {
    App app;

    std::cout << "--- add 3 tasks ---\n";
    const auto id1{app.addTask("Architecture plan")};
    const auto id2{app.addTask("Create event bus")};
    const auto id3{app.addTask("actions -> commands")};
    app.printState();

    std::cout << std::format("--- completing the task {} ---\n", id2);
    app.completeTask(id2);
    app.printState();
    std::cout << std::format("--- completed count {} ---\n", app.completedCount());

    std::cout << std::format("--- deleting task #{} ---\n", id1);
    app.removeTask(id1);
    app.printState();

    std::cout << "--- undo ---\n";
    app.undo();
    app.printState();

    std::cout << "--- undo ---\n";
    app.undo();
    app.printState();
    std::cout << std::format("--- completed count {} ---\n", app.completedCount());

    std::cout << "--- redo ---\n";
    app.redo();
    app.printState();
    std::cout << std::format("--- completed count {} ---\n", app.completedCount());

    (void) id3;

    return 0;
}
