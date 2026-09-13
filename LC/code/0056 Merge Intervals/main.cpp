#include <iostream>
#include <vector>
#include <algorithm>

namespace {

    std::vector<std::vector<int>> merge(const std::vector<std::vector<int>>& intervals) {
        if (intervals.empty()) return {};

        auto intervals_copy{intervals};
        std::ranges::sort(
            intervals_copy,
            [](const std::vector<int>& a, const std::vector<int>& b) {
                return a[0] < b[0];
            }
        );

        std::vector<std::vector<int>> result;
        result.push_back(intervals_copy[0]);

        for (int i{1}; i < static_cast<int>(intervals_copy.size()); ++i) {
            auto& current{intervals_copy[i]};

            if (auto& last_merged{result.back()};
                current[0] <= last_merged[1]) {
                last_merged[1] = std::max(last_merged[1], current[1]);
            } else {
                result.push_back(current);
            }
        }

        return result;
    }

    void print_intervals(const std::vector<std::vector<int>>& v, const std::string& label) {
        std::cout << std::format("[{}] (", label);
        std::string delimiter;
        for (int i{}; i < static_cast<int>(v.size()); ++i) {
            std::cout << std::format("{}[{}, {}]", delimiter, v[i][0], v[i][1]);
            delimiter = ", ";
        }
        std::cout << ")\n";
    }

    bool equal_intervals(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b) {
        return a == b;
    }

    void run_test(const std::string& desc,
                  const std::vector<std::vector<int>>& intervals,
                  const std::vector<std::vector<int>>& expected) {
        const auto result{merge(intervals)};
        const bool ok{equal_intervals(result, expected)};
        print_intervals(result, desc);
        print_intervals(expected, desc + "_expected");
        std::cout << std::format(" -> {}\n", ok ? "OK" : "FAIL");
    }

}

int main() {
    run_test("Classical example", {{1,3},{2,6},{8,10},{15,18}}, {{1,6},{8,10},{15,18}});
    run_test("Close contact", {{1,4},{4,5}}, {{1,5}});
    run_test("Non-sorted input", {{5,6},{1,3},{2,4}}, {{1,4},{5,6}});
    run_test("Complete absorption of one interval by another", {{1,10},{2,3},{4,5}}, {{1,10}});
    run_test("Without interceptions", {{1,2},{3,4},{5,6}}, {{1,2},{3,4},{5,6}});
    run_test("One interval", {{1,4}}, {{1,4}});
    run_test("All to one", {{1,4},{2,5},{3,6},{4,7}}, {{1,7}});

    return 0;
}
