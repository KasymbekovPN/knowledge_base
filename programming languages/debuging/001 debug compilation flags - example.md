---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debuging/_|<=]]

```cpp
#include <coroutine>  
#include <optional>  
#include <iostream>  
#include <format>  
  
// Простой Generator<T> — типичный пример из твоих заметок по корутинам  
template<typename T>  
struct Generator {  
    struct promise_type {  
        T current_value;  
  
        Generator get_return_object() {  
            return Generator{  
                std::coroutine_handle<promise_type>::from_promise(*this)  
            };        }        std::suspend_always initial_suspend() { return {}; }  
        std::suspend_always final_suspend() noexcept { return {}; }  
        std::suspend_always yield_value(T _value) {  
            current_value = _value;  
            return {};  
        }        void return_void() {}  
        void unhandled_exception() { std::terminate(); }  
    };  
    using handle_type = std::coroutine_handle<promise_type>;  
    handle_type coro;  
  
    explicit Generator(handle_type _coro) : coro(_coro) {}  
    ~Generator() { if (coro) coro.destroy(); }  
    Generator(Generator&&) = delete;  
    Generator(const Generator&) = delete;  
  
    bool next() {  
        coro.resume();  
        return !coro.done();  
    }  
    T value() const { return coro.promise().current_value; }  
};  
  
// Генератор чисел Фибоначчи — суспендится на каждом yield  
Generator<long long> fibonacci(int _count) {  
    long long a{0}, b{1};  
    for (int i{}; i < _count; ++i) {  
        co_yield a;  
        const long long next{a + b};  
        a = b;  
        b = next;  
    }}  
  
int main() {  
    auto gen = fibonacci(10);  
    long long sum{0};  
    while (gen.next()) {  
        long long v{gen.value()};  
        std::cout << std::format("{} ", v);  
        sum += v;  
    }    std::cout << std::format("\nSum: {}", sum);  
  
    return 0;  
}  
  
/*  
  
g++ -std=c++20 -g3 -gdwarf-5 -O0 coro_demo.cpp -o gdb_coro_O0  
g++ -std=c++20 -g3 -gdwarf-5 -Og coro_demo.cpp -o gdb_coro_Og  
clang++ -std=c++20 -g -O0 -gdwarf-4 -fuse-ld=lld coro_demo.cpp -o coro_clang_O0.exe  
clang++ -std=c++20 -g -Og -gdwarf-4 -fuse-ld=lld coro_demo.cpp -o coro_clang_Og.exe  
  
###  
  
C:\msys64\ucrt64\bin\gdb.exe gdb_coro_O0.exe  
break coro_demo.cpp:48  
run  
info locals      # увидишь a, b, i, next + служебные _Coro_* поля  
bt                # backtrace  
continue          # следующая итерация  
print a  
  
gdb_coro_O0.exe  
gdb_coro_Og.exe  
  
###  
  
C:\msys64\ucrt64\bin\gdb.exe -q -batch -ex "break coro_demo.cpp:48" -ex "run" -ex "info locals" -ex "bt" -ex "continue" -ex "print a" ./gdb_coro_O0  
C:\msys64\ucrt64\bin\gdb.exe -q -batch -ex "break coro_demo.cpp:48" -ex "run" -ex "info locals" -ex "bt" -ex "continue" -ex "print a" ./gdb_coro_Og  
  
###  
  
lldb ./coro_clang_O0  
lldb ./coro_clang_Og  
breakpoint set --file coro_demo.cpp --line 48  
run  
frame variable   # аналог "info locals", но с формой data formatter  
bt  
continue  
print a  
  
###  
  
cat > lldb_cmds.txt << 'EOF'  
breakpoint set --file coro_demo.cpp --line 48  
run  
frame variable  
bt  
continue  
continue  
continue  
print a  
print b  
print i  
quit  
EOF  
  
lldb -b -s lldb_cmds.txt ./coro_clang_O0  
lldb -b -s lldb_cmds.txt ./coro_clang_Og  
  
 */
```

## Практический вывод для тебя

| |`-O0`|`-Og`|
|---|---|---|
|Видимость твоих переменных|100%|100% (в этом примере)|
|Видимость служебных полей корутины|100%|Частично `<optimized out>`|
|Backtrace через libstdc++/промежуточные вызовы|Отдельные честные фреймы|Инлайнинг может схлопывать фреймы (LLDB это явно показывает, GDB — не всегда)|
|Скорость выполнения|Медленно|Заметно быстрее|

Для отладки **своей** логики корутины `-Og` вполне безопасен. Но если тебе нужно залезть именно в детали work state machine самой корутины (например, разбираться в баге компилятора или в том, как именно устроен suspend/resume) — бери `-O0`, там сохраняется максимум информации о служебных полях.
