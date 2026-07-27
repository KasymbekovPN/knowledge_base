// Command pattern: действия приложения как объекты.
// Демонстрация: undo/redo стек + макрос (несколько команд как одна операция отмены).

#include <iostream>
#include <format>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Receiver — объект, над которым команды реально выполняют работу.
// Сам Document ничего не знает про undo/redo, историю, макросы - это
// обычный класс с прямыми операциями. Вся "умность" про отмену действий
// живёт в командах, а не в Document - это и есть смысл паттерна: логика
// "как отменить" инкапсулирована рядом с логикой "как сделать".
// ---------------------------------------------------------------------------
class Document {
public:
    void insert(const size_t pos, const std::string& text) {
        if (pos > content_.size()) throw std::out_of_range("insert pos");
        content_.insert(pos, text);
    }

    void erase(const size_t pos, const size_t length) {
        if (pos + length > content_.size()) throw std::out_of_range("erase pos");
        content_.erase(pos, length);
    }

    const std::string& text() const { return content_; }

private:
    std::string content_;
};

// ---------------------------------------------------------------------------
// Command interface — контракт: любое действие приложения умеет
// выполниться и отмениться. Invoker (CommandManager) работает только
// с этим интерфейсом, не зная, что конкретно за действие внутри.
// ---------------------------------------------------------------------------
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

// ---------------------------------------------------------------------------
// Конкретная команда: вставка текста.
// ---------------------------------------------------------------------------
class InsertCommand : public ICommand {
public:
    InsertCommand(Document& doc, const size_t pos, const std::string& text):
        doc_{doc}, pos_{pos}, text_{std::move(text)} {}
    void execute() override { doc_.insert(pos_, text_); }
    void undo() override { doc_.erase(pos_, text_.size()); }
    std::string description() const override {
        return std::format("Insert(\"{}\" @{})", text_, pos_);
    }

private:
    Document& doc_;
    size_t pos_;
    std::string text_;
};

// ---------------------------------------------------------------------------
// Конкретная команда: удаление текста.
// Важный момент: чтобы undo() умел восстановить текст, команда обязана
// запомнить состояние ДО выполнения - это её приватное поле, никто извне
// про removedText_ не знает.
// ---------------------------------------------------------------------------
class DeleteCommand : public ICommand {
public:
    DeleteCommand(Document& doc, const size_t pos, const size_t length):
        doc_{doc}, pos_{pos}, length_{length} {}
    void execute() override {
        removed_text_ = doc_.text().substr(pos_, length_);
        doc_.erase(pos_, length_);
    }
    void undo() override { doc_.insert(pos_, removed_text_); }
    std::string description() const override {
        return std::format("Delete({} chars @{})", length_, pos_);
    }

private:
    Document& doc_;
    size_t pos_;
    size_t length_;
    // состояние, нужное для undo
    std::string removed_text_;
};

// ---------------------------------------------------------------------------
// MacroCommand — Composite поверх ICommand: несколько команд ведут себя
// как одна. Execute идёт по порядку, undo — В ОБРАТНОМ порядке (иначе
// вторая undo может оперировать над данными, которых первая undo ещё
// не восстановила - классическая ошибка при реализации макросов).
// ---------------------------------------------------------------------------
class MacroCommand : public ICommand {
public:
    void add(std::unique_ptr<ICommand> command) { commands_.push_back(std::move(command)); }
    void execute() override { for (const auto& cmd: commands_) cmd->execute(); }
    void undo() override {
        for (auto it{commands_.rbegin()}; it != commands_.rend(); ++it) (*it)->undo();
    }
    std::string description() const override {
        return std::format("Macro({}) commands", commands_.size());
    }

private:
    std::vector<std::unique_ptr<ICommand>> commands_;
};

// ---------------------------------------------------------------------------
// Invoker — хранит историю выполненных команд, управляет undo/redo.
// Не знает НИЧЕГО про Document, InsertCommand, DeleteCommand - работает
// только с ICommand. Это и есть развязка: добавить новый тип команды
// (например MoveCursorCommand) не требует трогать CommandManager вообще.
// ---------------------------------------------------------------------------
class CommandManager {
public:
    void execute(std::unique_ptr<ICommand> cmd) {
        std::cout << std::format(" -> execute {}\n", cmd->description());
        cmd->execute();
        undo_stack_.push(std::move(cmd));
        // новое действие обнуляет redo-хвост
        while (!redo_stack_.empty()) redo_stack_.pop();
    }

    bool undo() {
        if (undo_stack_.empty()) return false;
        auto cmd = std::move(undo_stack_.top());
        undo_stack_.pop();
        std::cout << std::format(" -> undo {}\n", cmd->description());
        cmd->undo();
        redo_stack_.push(std::move(cmd));

        return true;
    }

    bool redo() {
        if (redo_stack_.empty()) return false;
        auto cmd = std::move(redo_stack_.top());
        redo_stack_.pop();
        std::cout << std::format(" -> redo {}\n", cmd->description());
        cmd->execute();
        undo_stack_.push(std::move(cmd));

        return true;
    }

private:
    std::stack<std::unique_ptr<ICommand>> undo_stack_;
    std::stack<std::unique_ptr<ICommand>> redo_stack_;
};

int main() {
    Document doc;
    CommandManager mgr;

    std::cout << "[1]\n";
    mgr.execute(std::make_unique<InsertCommand>(doc, 0, "Hello"));
    std::cout << std::format("  doc = \"{}\"\n", doc.text());

    std::cout << "[2]\n";
    mgr.execute(std::make_unique<InsertCommand>(doc, 5, " World"));
    std::cout << "     doc = \"" << doc.text() << "\"\n";

    // Макрос: удалить "World" и сразу вставить "!!!" -> для пользователя это
    // одно логическое действие ("заменить"), и Ctrl+Z должен откатить его целиком.
    std::cout << "[3] macro (delete \" World\" + insert \"!!!\")\n";
    auto macro = std::make_unique<MacroCommand>();
    macro->add(std::make_unique<DeleteCommand>(doc, 5, 6));
    macro->add(std::make_unique<InsertCommand>(doc, 5, "!!!"));
    mgr.execute(std::move(macro));
    std::cout << "     doc = \"" << doc.text() << "\"\n";

    std::cout << "[4] undo (Ctrl+Z)\n";
    mgr.undo();
    std::cout << "     doc = \"" << doc.text() << "\"\n";

    std::cout << "[5] undo (insert \" World\")\n";
    mgr.undo();
    std::cout << "     doc = \"" << doc.text() << "\"\n";

    std::cout << "[6] redo (insert \" World\")\n";
    mgr.redo();
    std::cout << "     doc = \"" << doc.text() << "\"\n";

    return 0;
}
