#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

namespace {

struct Interval {
    int start;
    int end;  // start <= end, both ends are included
};

bool operator==(const Interval& a, const Interval& b) {
    return a.start == b.start && a.end == b.end;
}

// Merge all overlapping intervals and return the result sorted by start.
// Intervals are closed: {1, 4} and {4, 5} overlap (they share the point 4) and become {1, 5}.
std::vector<Interval> merge_intervals(std::vector<Interval> intervals) {
	std::ranges::sort(intervals, [](const Interval& a, const Interval& b) {
	    if (a.start == b.start) {
	        return a.end < b.end;
	    }
	    return a.start < b.start;
	});
	std::vector<Interval> result;

	for (const auto& interval: intervals) {
		if (!result.empty() && result.back().end >= interval.start) {
			result.back().end = std::max(interval.end, result.back().end);
		} else {
			result.push_back(interval);
		}
	}

    return result;
}

// Checker: repeated pairwise merging for small tests, sort + sweep for big ones.
std::vector<Interval> naive(std::vector<Interval> intervals) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (std::size_t i = 0; i < intervals.size() && !changed; ++i) {
            for (std::size_t j = i + 1; j < intervals.size() && !changed; ++j) {
                if (intervals[i].start <= intervals[j].end && intervals[j].start <= intervals[i].end) {
                    intervals[i].start = std::min(intervals[i].start, intervals[j].start);
                    intervals[i].end = std::max(intervals[i].end, intervals[j].end);
                    intervals.erase(intervals.begin() + static_cast<std::ptrdiff_t>(j));
                    changed = true;
                }
            }
        }
    }
    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) { return a.start < b.start; });
    return intervals;
}

std::vector<Interval> sweep(std::vector<Interval> intervals) {
    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) { return a.start < b.start; });
    std::vector<Interval> result;
    for (const Interval& cur : intervals) {
        if (!result.empty() && cur.start <= result.back().end) {
            result.back().end = std::max(result.back().end, cur.end);
        } else {
            result.push_back(cur);
        }
    }
    return result;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

void test_merge(const std::string& label, const std::vector<Interval>& intervals) {
    report(label, merge_intervals(intervals) == naive(intervals));
}

void test_big(const std::string& label, const std::vector<Interval>& intervals) {
    const std::vector<Interval> expected = sweep(intervals);
    const auto start = std::chrono::steady_clock::now();
    const std::vector<Interval> got = merge_intervals(intervals);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
    test_merge("description example 1",
        {{1, 3}, {2, 6}, {8, 10}, {15, 18}});
    test_merge("description example 2: touching intervals",
        {{1, 4}, {4, 5}});
    test_merge("empty",
        {});
    test_merge("one interval",
        {{5, 7}});
    test_merge("one point interval",
        {{3, 3}});
    test_merge("two disjoint",
        {{1, 2}, {4, 5}});
    test_merge("touching by one point only",
        {{1, 2}, {2, 2}});
    test_merge("adjacent but not touching",
        {{1, 2}, {3, 4}});
    test_merge("unsorted input",
        {{8, 10}, {1, 3}, {15, 18}, {2, 6}});
    test_merge("nested interval",
        {{1, 10}, {2, 3}, {4, 5}});
    test_merge("nested interval comes later",
        {{1, 10}, {2, 3}, {4, 12}, {6, 7}});
    test_merge("all merge into one",
        {{1, 4}, {0, 4}});
    test_merge("chain of overlaps",
        {{1, 3}, {3, 5}, {5, 7}, {7, 9}});
    test_merge("same start",
        {{2, 3}, {2, 8}, {2, 5}});
    test_merge("identical intervals",
        {{4, 6}, {4, 6}, {4, 6}});
    test_merge("negative values",
        {{-10, -5}, {-7, -1}, {0, 2}, {-3, 1}});
    test_merge("last interval merges with the previous one",
        {{1, 2}, {10, 20}, {15, 25}});
    test_merge("last interval stays alone",
        {{1, 5}, {2, 3}, {10, 11}});
    test_merge("extreme values",
        {{-2147483647 - 1, -2147483647 - 1}, {2147483646, 2147483647}, {2147483647, 2147483647}});
    test_merge("random 1: 6 intervals in 0..20",
        {{3, 7}, {15, 18}, {0, 2}, {6, 9}, {12, 12}, {17, 20}});
    test_merge("random 2: 10 intervals in -20..20",
        {{-5, 3}, {10, 12}, {-20, -15}, {-14, -8}, {4, 6}, {8, 9}, {-3, 0}, {15, 20}, {-9, -9}, {11, 14}});
    test_merge("random 3: 20 short intervals in 0..50",
        {{5, 8}, {40, 42}, {1, 3}, {20, 22}, {30, 33}, {2, 4}, {45, 49}, {9, 10}, {23, 25}, {35, 36},
         {11, 13}, {26, 28}, {41, 44}, {14, 16}, {6, 6}, {47, 50}, {18, 19}, {29, 31}, {37, 39}, {0, 0}});

    // ---- big tests ----
    {
        std::vector<Interval> v;
        for (int i = 0; i < 1000000; ++i) v.push_back({i * 3, i * 3 + 1});
        test_big("big: 1000000 disjoint intervals in sorted order", v);
    }
    {
        std::vector<Interval> v;
        for (int i = 999999; i >= 0; --i) v.push_back({i * 3, i * 3 + 1});
        test_big("big: 1000000 disjoint intervals in reverse order", v);
    }
    {
        std::vector<Interval> v;
        for (int i = 0; i < 1000000; ++i) v.push_back({i, i + 1});
        test_big("big: 1000000 touching intervals, one result", v);
    }
    {
        std::vector<Interval> v;
        unsigned x = 4242u;
        for (int i = 0; i < 1000000; ++i) {
            x = x * 1103515245u + 12345u;
            const int a = static_cast<int>((x >> 8) % 2000000000u);
            x = x * 1103515245u + 12345u;
            const int len = static_cast<int>((x >> 8) % 3000u);
            v.push_back({a, a + len});
        }
        test_big("big: 1000000 pseudo-random short intervals", v);
    }
    {
        std::vector<Interval> v;
        for (int i = 0; i < 1000000; ++i) v.push_back({i, 1000000000});
        test_big("big: 1000000 intervals with a common end", v);
    }
}

}  // namespace

int main() {
    test_all();
    return 0;
}