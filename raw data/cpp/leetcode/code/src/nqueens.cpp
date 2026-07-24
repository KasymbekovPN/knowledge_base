#include "nqueens.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <iostream>
#include <format>

namespace nqueens {

void backtrack(const int row,
                const int n,
                std::vector<int>& queens,
                std::unordered_set<int>& cols,
                std::unordered_set<int>& diag1,
                std::unordered_set<int>& diag2,
                std::vector<std::vector<std::string>>& result) {
    if (row == n) {
        // построить доску из queens и сохранить
        std::vector<std::string> board(n, std::string(n, '.'));
        for (int r{}; r < n; ++r) {
            board[r][queens[r]] = 'Q';
        }
        result.push_back(std::move(board));
        return;
    }

    for (int col{}; col < n; ++col) {
        if (cols.contains(col) || diag1.contains(row - col) || diag2.contains(row + col)) {
            continue; // клетка бьётся уже поставленным ферзём
        }

        // размещаем ферзя
        queens[row] = col;
        cols.insert(col);
        diag1.insert(row - col);
        diag2.insert(row + col);

        backtrack(row + 1, n, queens, cols, diag1, diag2, result);

        // откат
        cols.erase(col);
        diag1.erase(row - col);
        diag2.erase(row + col);
    }
}

std::vector<std::vector<std::string>> solve_nqueens(const int n) {
    std::vector<std::vector<std::string>> result;
    std::vector<int> queens(n, -1);
    std::unordered_set<int> cols, diag1, diag2;
    backtrack(0, n, queens, cols, diag1, diag2, result);

    return result;
}

void backtrack_bitmask(const int row,
                        const int n,
                        const int cols,
                        const int diag1,
                        const int diag2,
                        int& count) {
    if (row == n) {
        ++count;
        return;
    }

    // доступные позиции в этой строке: биты, не занятые ни cols, ни diag1, ни diag2
    int available{((1 << n) - n) & ~(cols | diag1 | diag2)};
    while (available) {
        // выделяем младший установленный бит
        const int bit{available & (-available)};
        available = -bit;

        backtrack_bitmask(row + 1, n, cols | bit, (diag1 | bit) << 1, (diag2 | bit) >> 1, count);
    }
}

void demo() {
    // int count{};
    // backtrack_bitmask(0, 4, 0, 0, 0, count);
    // std::cout << std::format("count: {}\n", count);

    for (const auto result = solve_nqueens(4);
        const auto& vec : result) {
        for (const auto& item : vec) {
            std::cout << std::format("{} \n", item);
        }
        std::cout << '\n';
    }
}

}