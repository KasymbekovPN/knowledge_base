
// ============================================================
// ЗАДАЧА: TextBuffer с Undo/Redo
//
// Реализовать структуру данных, представляющую текстовый буфер
// (как в простом текстовом редакторе), поддерживающую базовое
// редактирование и ОТМЕНУ последних изменений.
//
// Типичный сценарий: движок текстового редактора -- пользователь
// печатает, удаляет текст, жмёт Ctrl+Z и ожидает вернуться к
// предыдущему состоянию.
//
// Минимальный контракт:
//   - insert(pos, text)  -- вставить строку text в позицию pos
//   - erase(pos, count)  -- удалить count символов с позиции pos
//   - get_text()          -- вернуть текущее содержимое буфера
//   - undo()               -- отменить последнее изменение
//
// ВСЁ ОСТАЛЬНОЕ -- открытые вопросы. Прежде чем писать тесты и
// реализацию, стоит их прояснить, а не додумывать самостоятельно.
// Некоторые направления, где условие намеренно неполно:
//   - Что считается ОДНОЙ "единицей" отмены -- каждый отдельный
//     вызов insert()/erase(), или несколько последовательных
//     insert() подряд (как при обычном наборе текста) группируются
//     в одну undo-операцию (как в большинстве реальных редакторов)?
//   - Нужен ли redo? Если да -- что происходит с redo-историей,
//     если после undo() сделать НОВОЕ изменение (стандартно --
//     redo-стек очищается, но это не сказано явно)?
//   - Есть ли предел на глубину истории undo (аналог capacity в
//     LRU), или история растёт неограниченно?
//   - insert(pos, text) / erase(pos, count) с pos/count ВНЕ границ
//     текущего текста -- что происходит? Исключение? Зажимается
//     до валидного диапазона? Undefined behavior?
//   - undo() когда истории нет (ничего отменять) -- no-op? Возврат
//     bool "было ли что отменять"? Исключение?
//   - Нужно ли восстанавливать ПОЗИЦИЮ КУРСОРА при undo, или буфер
//     отвечает только за текст, а курсор -- забота вызывающего кода?
//   - Нужна ли потокобезопасность?
//   - Как измеряется "позиция" в тексте -- индекс символа (не байта,
//     если в тексте многобайтовые UTF-8 символы)? Задача предполагает
//     ASCII, или нужно думать про Unicode?
// ============================================================

#include <string>
#include <iostream>
#include <cassert>
#include <expected>
#include <deque>
#include <vector>

// ------------------------------------------------------------
// TODO: реализация класса TextBuffer
// ------------------------------------------------------------

class TextBuffer {
    static inline int DEFAULT_DEPTH{10};

    enum class Code {
        GOOD,
        BAD
    };

    struct IAction {
        virtual ~IAction() = default;
        virtual std::expected<IAction*, Code> execute(std::string& text) = 0;
    };

    struct InsertAction: IAction {
        size_t pos_;
        std::string text_;

        explicit InsertAction(const size_t pos, std::string text) : pos_(pos), text_(std::move(text)) {}

        std::expected<IAction*, Code> execute(std::string& text) override {
            if (text_.empty()) return std::unexpected(Code::BAD);
            if (pos_ > text.size()) return std::unexpected(Code::BAD);

            text.insert(pos_, text_);

            return new EraseAction(pos_, text.size());
        }
    };

    struct EraseAction: IAction {
        size_t pos;
        size_t count;

        explicit EraseAction(const size_t pos, const size_t count): pos(pos), count(count) {}

        std::expected<IAction *, Code> execute(std::string& text) override {
            if (count == 0) return std::unexpected(Code::BAD);
            if (pos > text.size()) return std::unexpected(Code::BAD);

            const auto erased{text.substr(pos, count)};
            text.erase(pos, count);

            return new InsertAction(pos, erased);
        }
    };

    std::string text_;
    std::deque<IAction*> undo_stack_{};
    int undo_depth_{DEFAULT_DEPTH};

public:
    class Builder {
        std::string text_{};
        int undo_depth_{DEFAULT_DEPTH};

        Builder() = default;

    public:
        static Builder create() { return {}; }

        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;
        Builder(Builder&&) = delete;
        Builder& operator=(Builder&&) = delete;

        Builder* text(std::string text) {
            text_ = std::move(text);
            return this;
        }

        Builder* undo_depth(const int depth) {
            undo_depth_ = depth > 0 ? depth : DEFAULT_DEPTH;
            return this;
        }

        TextBuffer build() {
            TextBuffer buffer(text_, undo_depth_);
            return buffer;
        }
    };

private:

    void new_to_stack(IAction* action) {
        undo_stack_.push_front(action);
        if (undo_stack_.size() > undo_depth_) {
            const auto p{undo_stack_.back()};
            undo_stack_.pop_back();
            delete p;
        }
    }

    explicit TextBuffer(std::string text, const int depth):
        text_{std::move(text)}, undo_depth_(depth){
    }

public:

    ~TextBuffer() {
        while (!undo_stack_.empty()) {
            const auto p{undo_stack_.front()};
            undo_stack_.pop_front();
            delete p;
        }
    }

    // TODO: insert(pos, text) -- вставить текст в позицию pos
    void insert(const size_t pos, const std::string& text) {
        InsertAction action{pos, text};
        if (const auto result{action.execute(text_)};
            result.has_value()) {
            new_to_stack(result.value());
        }
    }

    // TODO: erase(pos, count) -- удалить count символов с позиции pos
    void erase(const size_t pos, const size_t count) {
        EraseAction action{pos, count};
        if (const auto result{action.execute(text_)};
            result.has_value()) {
            new_to_stack(result.value());
        }
    }

    // TODO: get_text() -- вернуть текущее содержимое буфера
    [[nodiscard]] std::string get_text() const {
        return text_;
    }

    // TODO: undo() -- отменить последнее изменение.
    // Возвращает bool? Что означает true/false -- уточнить контракт.
    bool undo() {
        if (undo_stack_.empty()) return false;

        const auto action{undo_stack_.front()};
        undo_stack_.pop_front();

        const auto result{action->execute(text_)};
        delete action;
        if (result.has_value()) {
            delete result.value();
        }

        return true;
    }
};

// ------------------------------------------------------------
// Тесты -- содержимое закомментировано, т.к. реализации ещё нет.
// Раскомментировать и адаптировать по мере того, как контракт
// проясняется и реализация появляется.
// ------------------------------------------------------------

void test_basic_insert() {
    std::cout << "[test_basic_insert] ";
    auto buf{TextBuffer::Builder::create().build()};
    buf.insert(0, "hello");
    assert(buf.get_text() == "hello");
    buf.insert(5, " world");
    assert(buf.get_text() == "hello world");
    std::cout << "TODO\n";
}

void test_basic_erase() {
    std::cout << "[test_basic_erase] ";
    auto buf{TextBuffer::Builder::create().build()};
    buf.insert(0, "hello world");
    buf.erase(5, 6); // удалить " world"
    assert(buf.get_text() == "hello");
    std::cout << "TODO\n";
}

void test_undo_after_insert() {
    std::cout << "[test_undo_after_insert] ";
    auto buf{TextBuffer::Builder::create().build()};
    buf.insert(0, "hello");
    buf.undo();
    assert(buf.get_text() == "");
    std::cout << "TODO\n";
}

void test_undo_after_erase() {
    std::cout << "[test_undo_after_erase] ";
    auto buf{TextBuffer::Builder::create().build()};
    buf.insert(0, "hello world");
    buf.erase(5, 6);
    buf.undo();
    assert(buf.get_text() == "hello world"); // erase отменён
    std::cout << "TODO\n";
}

void test_multiple_undo() {
    std::cout << "[test_multiple_undo] ";
    // Зависит от того, как группируются операции в единицы undo --
    // см. открытый вопрос в описании задачи. Пока условие не
    // прояснено, конкретные ожидания здесь под вопросом.

    auto buf{TextBuffer::Builder::create().build()};
    buf.insert(0, "a");
    buf.insert(1, "b");
    buf.insert(2, "c");
    assert(buf.get_text() == "abc");
    buf.undo(); // отменяет ОДНУ единицу -- что именно она отменяет?
    assert(buf.get_text() == "ab");  // если каждый insert -- своя единица
    std::cout << "TODO\n";
}

void test_undo_with_empty_history() {
    std::cout << "[test_undo_with_empty_history] ";
    auto buf{TextBuffer::Builder::create().build()};
    bool result = buf.undo(); // истории нет -- что должно произойти?
    assert(result == false); // если контракт -- bool "было ли что отменять"
    assert(buf.get_text() == "");
    std::cout << "TODO\n";
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------

int main() {
    test_basic_insert();
    test_basic_erase();
    test_undo_after_insert();
    test_undo_after_erase();
    test_multiple_undo();
    test_undo_with_empty_history();

    std::cout << "\nВсе тестовые методы вызваны (содержимое ещё не реализовано).\n";
    return 0;
}