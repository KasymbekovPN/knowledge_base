#include <iostream>
#include <format>
#include <unordered_map>
#include <algorithm>
#include <ranges>

namespace {
    int least_interval(const std::vector<char>& tasks, const int n) {
        std::unordered_map<char, int> freq;
        for (char t: tasks) freq[t]++;

        int max_freq{};
        for (auto& task: freq | std::views::keys) {
            max_freq = std::max(max_freq, freq[task]);
        }

        int max_count{};
        for (const auto& count: freq | std::views::values) {
            if (count == max_freq) max_count++;
        }

        const int skeleton{(max_freq - 1) * (n + 1) + max_count};
        return std::max(skeleton, static_cast<int>(tasks.size()));
    }

    void run_test(const std::string& desc, const std::vector<char>& tasks, const int n, const int expected) {
        const int result{least_interval(tasks, n)};
        std::cout << std::format("desc: {}, expected: {}, result: {}, success: {}\n",
            desc,
            expected,
            result,
            result == expected ? "OK" : "FAIL");
    }
}

int main() {
    run_test("Classical example: AAA BBB, n=2", {'A', 'A', 'A', 'B', 'B', 'B'}, 2, 8);
    run_test("n=0, cooldown not need", {'A', 'A', 'A', 'B', 'B', 'B'}, 0, 6);
    run_test("One task is dominated", {'A','A','A','A','A','A','B','C','D','E','F','G'}, 2, 16);
    run_test("Several tasks have same max. freq.", {'A','A','A','B','B','B','C','C','C','D','D','E'}, 2, 12);
    run_test("Alone task", {'A'}, 5, 1);
    run_test("Large n relative to the volume of tasks", {'A', 'A', 'B', 'B'}, 10, 13);

    return 0;
}
