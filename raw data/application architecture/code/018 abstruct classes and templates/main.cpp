// Runtime polymorphism (виртуальные функции) vs compile-time polymorphism
// (шаблоны) - измеряем реальную разницу, а не полагаемся на общие слова.
//
// Важная оговорка про честность сравнения: virtualShapes чередует Circle/
// Square, чтобы вызов через vtable был РЕАЛЬНО полиморфным (megamorphic
// call site) - иначе компилятор мог бы догадаться о типе и девиртуализировать
// сам. staticShapes же вынужденно однородны (все CircleStatic) - именно
// потому что std::vector<Shape> инстанцируется под ОДИН конкретный тип.
// Это не баг замера, а иллюстрация самого компромисса: шаблоны не умеют
// держать разнородную коллекцию, virtual - умеет.

#include <chrono>
#include <cstdio>
#include <memory>
#include <random>
#include <vector>

// ---------- Вариант 1: runtime polymorphism ----------
class IShape {
public:
    virtual ~IShape() = default;
    virtual double area() const = 0;
};

class Circle: public IShape {
public:
    explicit Circle(const double r): r_{r} {}
    double area() const override {
        return 3.14159265358979 * r_ * r_;
    }
private:
    double r_{0.0};
};

class Square: public IShape {
public:
    explicit Square(const double s): s_{s} {}
    double area() const override { return s_ * s_; }
private:
    double s_{0.0};
};

double sumAreasVirtual(const std::vector<std::unique_ptr<IShape>>& shapes) {
    double total{0.0};
    for (const auto& s: shapes) total += s->area();

    return total;
}

// ---------- Вариант 2: compile-time polymorphism ----------
struct CircleStatic {
    double r;
    double area() const { return 3.14159265358979 * r * r; }
};

struct SquareStatic {
    double s;
    double area() const { return s * s; }
};

template <typename Shape>
double sumAreasStatic(const std::vector<Shape>& shapes) {
    double total{0.0};
    // тип известен на этапе компиляции
    for (const auto& s: shapes) total += s.area();

    return total;
}

int main() {
    constexpr int N{20'000'000};

    std::vector<std::unique_ptr<IShape>> virtualShapes;
    std::vector<CircleStatic> staticShapes;
    virtualShapes.reserve(N);
    staticShapes.reserve(N);

    std::mt19937 rng{42};
    std::uniform_real_distribution<double> distribution{0.1, 10.0};
    for (int i{}; i < N; ++i) {
        const double R{distribution(rng)};
        if (i % 2 ) {
            virtualShapes.push_back(std::make_unique<Circle>(R));
        } else {
            virtualShapes.push_back(std::make_unique<Square>(R));
        }
        staticShapes.push_back(CircleStatic{R});
    }

    const auto T0{std::chrono::steady_clock::now()};
    const double TOTAL_VIRTUAL{sumAreasVirtual(virtualShapes)};
    const auto T1{std::chrono::steady_clock::now()};
    const double TOTAL_STATIC{sumAreasStatic(staticShapes)};
    const auto T2{std::chrono::steady_clock::now()};

    const double virtual_ms{std::chrono::duration<double, std::milli>(T1 - T0).count()};
    const double static_ms{std::chrono::duration<double, std::milli>(T2 - T1).count()};

    std::printf("virtual dispatch: %8.2f ms  (sum=%.3f)\n", virtual_ms, TOTAL_VIRTUAL);
    std::printf("static  dispatch: %8.2f ms  (sum=%.3f)\n", static_ms, TOTAL_STATIC);
    std::printf("speedup:          %8.2fx\n", virtual_ms / static_ms);

    return 0;
}
