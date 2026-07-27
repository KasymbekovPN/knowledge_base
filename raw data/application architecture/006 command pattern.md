---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            "name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {            "name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }    ],    "buildPresets": [  
        {            "name": "debug",  
            "configurePreset": "debug"  
        }  
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(app CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
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
```

Идея простая: вместо того чтобы вызывать `doc.insert(...)` напрямую из обработчика кнопки, действие оборачивается в объект с методами `execute()`/`undo()`. Как только у вас "действие" — это объект, а не просто вызов функции, с ним можно делать вещи, которые с обычным вызовом сделать нельзя: положить в стек и отменить, записать в очередь и выполнить позже, сериализовать и отправить по сети, сгруппировать несколько действий в одно.

В примере четыре роли, и важно, что они не знают друг о друге больше необходимого:

`Document` (receiver) — обычный класс с прямыми операциями, ничего не знает о существовании undo/redo. `ICommand` — контракт "выполниться / отмениться". `InsertCommand`/`DeleteCommand` — конкретные действия, каждое само решает, что ему нужно запомнить, чтобы отмена сработала (`DeleteCommand` перед удалением сохраняет `removedText_` — без этого `undo()` был бы нечем восстанавливать). `CommandManager` (invoker) работает только с интерфейсом `ICommand`, вообще не зная про `Document`, `InsertCommand` или что-либо конкретное — добавить новый тип команды (`MoveCursorCommand`, `FormatCommand`) не требует трогать сам менеджер.

Undo/redo механически — два стека. `execute()` кладёт команду в undo-стек и обнуляет redo-стек (иначе после нового действия старая "будущая" ветка истории потеряла бы смысл — классический баг, если это забыть). `undo()` снимает команду с undo-стека, откатывает и перекладывает в redo-стек; `redo()` — зеркально. Именно поэтому команда обязана быть самодостаточной: она носит с собой всё состояние, нужное для отмены, а не полагается на то, что кто-то снаружи это состояние сохранит.

Macro (`MacroCommand`) — Composite поверх `ICommand`: список команд ведёт себя как одна. Ключевая деталь, которую легко упустить — `undo()` идёт в обратном порядке относительно `execute()`. В примере макрос сначала удаляет " World", потом вставляет "!!!"; при отмене нужно сначала откатить вставку "!!!", и только потом восстановить " World" — иначе позиции (`pos_`) в командах разъедутся относительно текущего состояния документа. Тест это подтверждает: `undo()` макроса одним вызовом возвращает документ в состояние до обеих операций сразу — то, что пользователь ожидает от одного Ctrl+Z после "replace".

Как это связано с архитектурой приложения в целом, а не просто с текстовым редактором: Command — это способ вынести "что можно сделать в приложении" в отдельный, переиспользуемый слой, отвязанный от того, кто инициирует действие. Один и тот же `InsertCommand` может быть создан из клика мыши, из горячей клавиши, из скрипта автоматизации или из сетевого сообщения — вызывающему коду всё равно, `CommandManager` просто исполняет `ICommand`. Это же открывает дверь к очереди команд: если создать команду в одном потоке (например, обработчик сети), а выполнить её нужно в потоке event loop'а (вспоминая наш пример с `io_context`) — можно `post()` команду в `io_context` вместо того, чтобы городить мьютексы вокруг `Document` напрямую. Команда как объект — естественная единица передачи работы между потоками/подсистемами, а не просто механизм undo.
