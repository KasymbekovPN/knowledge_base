#include <chrono>
#include <boost/asio.hpp>
#include <iostream>
#include <format>
#include <memory>

class TimerChain: public std::enable_shared_from_this<TimerChain> {
public:
    TimerChain(boost::asio::io_context& io, int id):
        timer_{io},
        id_{id},
        tick_count_{0} {}

    void start() { schedule_next(); }
private:
    void schedule_next() {
        timer_.expires_after(std::chrono::milliseconds(100));
        auto self = shared_from_this();
        timer_.async_wait([self](const boost::system::error_code& ec) {
            self->on_timer(ec); // <-- breakpoint
        });
    }

    void on_timer(const boost::system::error_code& ec) {
        if (ec) {
            std::cout << std::format("timer {}, error: {}\n", id_, ec.message());
            return;
        }

        ++tick_count_;
        std::cout << std::format("timer {}, tick: {}\n", id_, tick_count_); // <-- breakpoint

        if (tick_count_ < 3) { schedule_next(); }
    }

    boost::asio::steady_timer timer_;
    int id_;
    int tick_count_;
};

int main() {
    boost::asio::io_context io;

    auto chain_a = std::make_shared<TimerChain>(io, 1);
    auto chain_b = std::make_shared<TimerChain>(io, 2);

    chain_a->start();
    chain_b->start();

    std::cout << "io_context.run() starting\n";
    io.run(); // <-- breakpoint
    std::cout << "io_context.run() finished, all handlers done\n";

    return 0;
}

/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 20
breakpoint set --file main.cpp --line 31
breakpoint set --file main.cpp --line 51
run
continue
print self.pointer
print *(TimerChain*)0x0000029ff7bfff70



###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:20
break main.cpp:31
break main.cpp:51
run

 */
