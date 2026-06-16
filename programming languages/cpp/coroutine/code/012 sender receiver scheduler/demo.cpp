// clang++ -std=c++26 -I stdexec/include my_async.cpp -o my_async
// & 'C:/Program Files/LLVM/bin/clang++.exe' -std=c++26 -I 'C:/projects/_3side/stdexec/include' 'C:\projects\knowledge_base\programming languages\cpp\coroutine\code\012 sender receiver scheduler\demo.cpp' -o 'C:\projects\knowledge_base\programming languages\cpp\coroutine\code\012 sender receiver scheduler/app.exe'

#include <iostream>
#include <format>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

namespace ex = stdexec;

int main() {
    exec::static_thread_pool pool{4};
    ex::scheduler auto sch = pool.get_scheduler();

    ex::sender auto work =
          ex::schedule(sch)
        | ex::then([]{ return 21; })
        | ex::then([](int x){ return x * 2; })
        | ex::then([](int x){
            std::cout << std::format("inner: {}\n", x);
            return x;
        });

    auto [r] = ex::sync_wait(work).value();
    std::cout << std::format("result: {}\n", r);
}
