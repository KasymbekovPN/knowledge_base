#include <iostream>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <unordered_set>
#include <functional>
#include <queue>
#include <array>
#include <chrono>

namespace {

// grid[r][c] is '1' (land) or '0' (water). An island is a group of land cells connected
// horizontally or vertically (diagonal neighbours are NOT connected).
// Return the number of islands. All rows have the same length. The grid may be empty.
int num_islands(const std::vector<std::string>& grid) {
	if (grid.empty() || grid[0].empty()) return 0;

	const std::size_t rows{grid.size()};
	const std::size_t cols{grid[0].size()};
	int islands{0};

	auto hash = [](const std::pair<std::size_t, std::size_t>& p) {
		const std::size_t h1{std::hash<std::size_t>{}(p.first)};
		const std::size_t h2{std::hash<std::size_t>{}(p.second)};

		return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
	};
	std::unordered_set<std::pair<std::size_t, std::size_t>, decltype(hash)> visited;
	std::array<std::pair<int, int>, 4> directions{
		{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}
	};

	for (std::size_t r{0}; r < rows; ++r) {
		for (std::size_t c{0}; c < cols; ++c) {
			if (grid[r][c] == '0' || visited.contains({r, c})) { continue; }

			++islands;

			std::queue<std::pair<std::size_t, std::size_t>> q;
			q.emplace(r, c);
			visited.emplace(r, c);

			while (!q.empty()) {
				auto [curr_r, curr_c] = q.front();
				q.pop();

				for (const auto& [dr, dc] : directions) {
					int next_r{static_cast<int>(curr_r) + dr};
					int next_c{static_cast<int>(curr_c) + dc};

					if (next_r >= 0 && next_r < static_cast<int>(rows) &&
						next_c >= 0 && next_c < static_cast<int>(cols)) {

						std::pair<std::size_t, std::size_t> next_pos{
						    static_cast<std::size_t>(next_r),
						    static_cast<std::size_t>(next_c)
						};

						if (grid[next_pos.first][next_pos.second] == '1' && !visited.contains(next_pos)) {
						    visited.insert(next_pos);
						    q.push(next_pos);
						}
					}
				}
			}
		}
	}

	return islands;
}

// Checker for small tests: union-find over the cells.
int islands_dsu(const std::vector<std::string>& grid) {
    if (grid.empty() || grid[0].empty()) return 0;
    const std::size_t rows = grid.size();
    const std::size_t cols = grid[0].size();
    std::vector<std::size_t> parent(rows * cols);
    for (std::size_t i = 0; i < parent.size(); ++i) parent[i] = i;
    const auto find = [&parent](std::size_t x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };
    for (std::size_t r = 0; r < rows; ++r)
        for (std::size_t c = 0; c < cols; ++c) {
            if (grid[r][c] != '1') continue;
            if (r + 1 < rows && grid[r + 1][c] == '1') parent[find(r * cols + c)] = find((r + 1) * cols + c);
            if (c + 1 < cols && grid[r][c + 1] == '1') parent[find(r * cols + c)] = find(r * cols + c + 1);
        }
    int count = 0;
    for (std::size_t r = 0; r < rows; ++r)
        for (std::size_t c = 0; c < cols; ++c)
            if (grid[r][c] == '1' && find(r * cols + c) == r * cols + c) ++count;
    return count;
}

// Checker for big tests: iterative BFS on a copy.
int islands_bfs(std::vector<std::string> grid) {
    if (grid.empty() || grid[0].empty()) return 0;
    const std::size_t rows = grid.size();
    const std::size_t cols = grid[0].size();
    int count = 0;
    std::vector<std::pair<std::size_t, std::size_t>> queue;
    for (std::size_t r = 0; r < rows; ++r)
        for (std::size_t c = 0; c < cols; ++c) {
            if (grid[r][c] != '1') continue;
            ++count;
            grid[r][c] = '0';
            queue.assign(1, {r, c});
            for (std::size_t head = 0; head < queue.size(); ++head) {
                const auto [y, x] = queue[head];
                const long long dy[4] = {1, -1, 0, 0};
                const long long dx[4] = {0, 0, 1, -1};
                for (int d = 0; d < 4; ++d) {
                    const long long ny = static_cast<long long>(y) + dy[d];
                    const long long nx = static_cast<long long>(x) + dx[d];
                    if (ny < 0 || nx < 0 || ny >= static_cast<long long>(rows) || nx >= static_cast<long long>(cols)) continue;
                    if (grid[static_cast<std::size_t>(ny)][static_cast<std::size_t>(nx)] != '1') continue;
                    grid[static_cast<std::size_t>(ny)][static_cast<std::size_t>(nx)] = '0';
                    queue.push_back({static_cast<std::size_t>(ny), static_cast<std::size_t>(nx)});
                }
            }
        }
    return count;
}

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

void test_islands(const std::string& label, const std::vector<std::string>& grid) {
    const std::vector<std::string> copy = grid;
    const bool ok = num_islands(grid) == islands_dsu(grid) && grid == copy;
    report(label, ok);
}

void test_big(const std::string& label, const std::vector<std::string>& grid) {
    const int expected = islands_bfs(grid);
    const auto start = std::chrono::steady_clock::now();
    const int got = num_islands(grid);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), got == expected && fast);
}

void test_all() {
    test_islands("description example 1",
        {"11110",
         "11010",
         "11000",
         "00000"});
    test_islands("description example 2",
        {"11000",
         "11000",
         "00100",
         "00011"});
    test_islands("empty grid",
        {});
    test_islands("one water cell",
        {"0"});
    test_islands("one land cell",
        {"1"});
    test_islands("one row",
        {"1011011"});
    test_islands("one column",
        {"1", "1", "0", "1", "0", "1", "1"});
    test_islands("all water",
        {"000", "000", "000"});
    test_islands("all land",
        {"111", "111", "111"});
    test_islands("diagonal cells are not connected",
        {"10", "01"});
    test_islands("diagonal chain",
        {"1000", "0100", "0010", "0001"});
    test_islands("checkerboard",
        {"1010", "0101", "1010", "0101"});
    test_islands("ring around water",
        {"11111", "10001", "10101", "10001", "11111"});
    test_islands("two islands touching a border",
        {"1001", "0000", "1001"});
    test_islands("snake",
        {"11111", "00001", "11111", "10000", "11111"});
    test_islands("comb",
        {"11111", "10101", "10101", "10101"});
    test_islands("island in the corners",
        {"10001", "00000", "00000", "10001"});
    test_islands("non-square 2x6",
        {"110011", "100001"});
    test_islands("non-square 6x2",
        {"11", "01", "00", "10", "11", "01"});
    test_islands("random 1: 4x5",
        {"10110", "01001", "11100", "00011"});
    test_islands("random 2: 6x6",
        {"101101", "010010", "111001", "000110", "101011", "010100"});
    test_islands("random 3: 8x10",
        {"1011010010", "0100101101", "1110010001", "0001101110", "1010111000", "0101000101", "1100110011", "0011001100"});

    // ---- big tests ----
    {
        std::vector<std::string> g(1000, std::string(1000, '1'));
        test_big("big: 1000x1000, all land (one island)", g);
    }
    {
        std::vector<std::string> g(1000, std::string(1000, '0'));
        test_big("big: 1000x1000, all water", g);
    }
    {
        std::vector<std::string> g(1000, std::string(1000, '0'));
        for (std::size_t r = 0; r < 1000; ++r)
            for (std::size_t c = 0; c < 1000; ++c)
                if ((r + c) % 2 == 0) g[r][c] = '1';
        test_big("big: 1000x1000, checkerboard (500000 islands)", g);
    }
    {
        std::vector<std::string> g(1000, std::string(1000, '0'));
        for (std::size_t r = 0; r < 1000; r += 2) {
            for (std::size_t c = 0; c < 1000; ++c) g[r][c] = '1';
            if (r + 2 < 1000) g[r + 1][(r / 2) % 2 == 0 ? 999 : 0] = '1';
        }
        test_big("big: 1000x1000, one long snake island", g);
    }
    {
        std::vector<std::string> g(1000, std::string(1000, '0'));
        unsigned x = 31337u;
        for (std::size_t r = 0; r < 1000; ++r)
            for (std::size_t c = 0; c < 1000; ++c) {
                x = x * 1103515245u + 12345u;
                if ((x >> 8) % 100u < 55u) g[r][c] = '1';
            }
        test_big("big: 1000x1000, random 55% land", g);
    }
    {
        std::vector<std::string> g(1, std::string(1000000, '1'));
        test_big("big: 1 x 1000000, one island", g);
    }
}

}  // namespace

int main() {
    test_all();
    return 0;
}