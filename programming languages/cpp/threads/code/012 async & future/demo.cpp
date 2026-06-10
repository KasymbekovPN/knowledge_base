#include <iostream>
#include <future>
#include <vector>
#include <numeric>

int sum(std::vector<int>::iterator _begin,
        std::vector<int>::iterator _end) {
    return std::accumulate(_begin, _end, 0);
}

int main() {
    std::vector<int> buffer(1000000, 1);
    auto&& mid = buffer.begin() + buffer.size() / 2;

    auto&& f0 = std::async(std::launch::async, sum, buffer.begin(), mid);
    auto&& f1 = std::async(std::launch::async, sum, mid, buffer.end());

    std::cout << f0.get() + f1.get() << std::endl;

    return 0;
}
