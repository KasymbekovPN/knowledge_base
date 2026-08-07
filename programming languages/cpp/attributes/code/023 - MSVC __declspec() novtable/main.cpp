#include <format>
#include <iostream>

#if defined(_MSC_VER)
    #define NOVTABLE __declspec(novtable)
#else
    #define NOVTABLE
#endif

namespace {
    class NOVTABLE IPlugin {
    public:
        virtual ~IPlugin() = default;
        virtual const char* name() const = 0;
        virtual void execute() const = 0;
    };

    class ConcretePlugin: public IPlugin {
    public:
        const char* name() const override { return "ConcretePlugin"; }
        void execute() const override { std::cout << "[OK]\n"; }
    };
}

int main() {
    const ConcretePlugin plugin;
    std::cout << std::format("name: {}\n", plugin.name());
    plugin.execute();

    return 0;
}
