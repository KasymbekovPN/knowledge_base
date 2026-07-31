// Type erasure: полиморфизм без общего базового класса. Три уровня -
// std::function (стирает вызываемые объекты), std::any (стирает вообще
// любой тип), и самодельная обёртка AnyDrawable (Concept/Model idiom -
// то, как std::function устроен внутри).

#include <any>
#include <functional>
#include <iostream>
#include <format>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// 1. std::function - стирает любой вызываемый объект с нужной сигнатурой
// ============================================================
double add(const double a, const double b) { return a + b; }

struct Multiplier {
    double factor;
    double operator()(const double a, const double b) const { return a * b * factor; }
};

void demoFunction() {
    std::cout << "--- std::function ---\n";

    const double CAPTURED{10.0};
    std::vector<std::function<double(double, double)>> ops;
    ops.push_back(add);
    ops.push_back([](const double a, const double b) { return a - b; });
    ops.push_back([CAPTURED](const double a, const double b) { return a + b + CAPTURED; });
    ops.push_back(Multiplier{2.0});

    // add, лямбды и Multiplier не имеют НИ ОДНОГО общего предка - но все
    // лежат в одном std::vector<std::function<...>> и вызываются одинаково.
    for (const auto& op : ops)
        std::cout << std::format("result: {}\n", op(3, 4));
}

// ============================================================
// 2. std::any - стирает вообще любой тип, не только вызываемые объекты
// ============================================================
void demoAny() {
    std::cout << "\n--- std::any ---\n";

    std::vector<std::any> bag;
    bag.push_back(42);
    bag.push_back(std::string("hello"));
    bag.push_back(3.14);

    for (const auto& v: bag) {
        if (auto* i = std::any_cast<int>(&v)) {
            std::cout << std::format("int: {}\n", *i);
        } else if (auto* s = std::any_cast<std::string>(&v)) {
            std::cout << std::format("string: {}\n", *s);
        } else if (auto* d = std::any_cast<double>(&v)) {
            std::cout << std::format("double: {}\n", *d);
        }
    }

    try {
        std::any_cast<int>(bag[1]);
    } catch (const std::bad_any_cast& e) {
        std::cerr << std::format("caught bad_any_cast: {}\n", e.what());
    }
}

template <typename T>
concept Drawable = requires(T value) { { value.draw() }; };

// ============================================================
// 3. Самодельная обёртка (Concept/Model idiom) - то, как std::function
// устроен внутри, только под конкретную нужную нам операцию draw().
// ============================================================
class AnyDrawable {
public:
    // Шаблонный конструктор - принимает ЛЮБОЙ тип T, у которого есть
    // метод draw(). Никакого общего интерфейса от T не требуется -
    // duck typing, проверяемый на этапе компиляции внутри Model<T>.
    template <Drawable T>
    AnyDrawable(T obj): self_{std::make_unique<Model<T>>(std::move(obj))} {}

    // unique_ptr<Concept> сам не копируется - копирование реализуем
    // через виртуальный clone().
    AnyDrawable(const AnyDrawable& other) : self_{other.self_->clone()} {}
    AnyDrawable& operator=(const AnyDrawable& other) {
        self_ = other.self_->clone();
        return *this;
    }

    AnyDrawable(AnyDrawable&& other) noexcept = default;
    AnyDrawable& operator=(AnyDrawable&& other) noexcept = default;

    void draw() const { self_->draw(); }

private:
    // Concept - приватный интерфейс, о существовании которого T ничего
    // не знает и никогда не узнает.
    struct Concept {
        virtual ~Concept() = default;
        virtual void draw() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    // Model<T> - единственное место, которое знает и про Concept, и про T.
    // Внутри всё равно самый обычный виртуальный вызов - type erasure не
    // отменяет стоимость диспетчеризации, она просто прячет её от T.
    template<typename T>
    struct Model : Concept {
        explicit Model(T obj): obj_(std::move(obj)) {}
        void draw() const override { obj_.draw(); }
        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model<T>>(obj_);
        }

        T obj_;
    };

    std::unique_ptr<Concept> self_;
};

// Три никак не связанных типа - ни общего предка, ни общего интерфейса,
// ни малейшего представления друг о друге.
struct Circle {
    double r;
    void draw() const {
        std::cout << std::format("Circle {}\n", r);
    }
};

struct SquareShape {
    double side;
    void draw() const {
        std::cout << std::format("SquareShape {}\n", side);
    }
};

// Представим, что это тип из чужой библиотеки, который мы не можем
// поменять - у него физически нет и не может появиться общий интерфейс
// с Circle/SquareShape.
struct ThirdPartyStar {
    int points;
    void draw() const {
        std::cout << std::format("ThirdPartyStar {}\n", points);
    }
};

void demoAnyDrawable() {
    std::cout << "\n--- AnyDrawable (custom type erasure) ---\n";

    std::vector<AnyDrawable> shapes;
    shapes.push_back(Circle{5.0});
    shapes.push_back(SquareShape{3.0});
    shapes.push_back(ThirdPartyStar{5});

    std::cout << "  originals\n";
    for (const auto& s: shapes) s.draw();

    // Копируем весь вектор целиком - проверяем, что clone() реально
    // работает и копии независимы от оригиналов.
    const std::vector<AnyDrawable> copies = shapes;
    std::cout << "  copies (via clone)\n";
    for (const auto& s: copies) s.draw();
}

int main() {
    demoFunction();
    demoAny();
    demoAnyDrawable();

    return 0;
}
