// Пробная единица трансляции: единственная цель - дать CLion/CMake реальный
// compile-context (флаги -std=c++23 и т.д.) для event_bus.hpp, у которого
// пока нет других потребителей (event_bus - INTERFACE-библиотека).
#include "event_bus/event_bus.hpp"

struct PingEvent {
    int value{};
};

int main() {
    EventBus bus;
    int received{0};
    auto connection = bus.subscribe<PingEvent>([&received](const PingEvent& e) { received = e.value; });
    bus.publish(PingEvent{42});
    return received == 42 ? 0 : 1;
}