---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

```cpp
#include <iostream>
#include <chrono>

struct Timer {
    std::chrono::steady_clock::time_point start;
    std::string name;

    Timer(const std::string& _name):
        name{_name},
        start{std::chrono::steady_clock::now()} {
        std::cout << "Timer created..." << std::endl;
    }

    ~Timer() {
        auto end = std::chrono::steady_clock::now();
        auto duration =
	        std::chrono::duration_cast<std::chrono::milliseconds>(
		        end - start
		    );
        std::cout << name << " took " << duration.count() << std::endl;
    }
};

void test_function();

int main() {
    test_function();

    return 0;
}

void test_function() {
    Timer t{"slow"};
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

```
Timer created...
slow took 106
```
