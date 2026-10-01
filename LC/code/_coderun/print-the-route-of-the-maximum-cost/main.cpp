/*

https://coderun.yandex.ru/problem/print-the-route-of-the-maximum-cost

В левом верхнем углу прямоугольной таблицы размером N×M находится черепашка. В каждой клетке таблицы записано
некоторое число. Черепашка может перемещаться вправо или вниз, при этом маршрут черепашки заканчивается
в правом нижнем углу таблицы.

Подсчитаем сумму чисел, записанных в клетках, через которую проползла черепашка (включая начальную и конечную клетку).
Найдите наибольшее возможное значение этой суммы и маршрут, на котором достигается эта сумма.

Формат ввода
В первой строке входных данных записаны два натуральных числа N и M, не превосходящих 100 — размеры таблицы.
Далее идет N строк, каждая из которых содержит M чисел, разделенных пробелами — описание таблицы.
Все числа в клетках таблицы целые и могут принимать значения от 0 до 100.

Формат вывода
Первая строка выходных данных содержит максимальную возможную сумму, вторая — маршрут, на котором достигается эта сумма.
Маршрут выводится в виде последовательности, которая должна содержать N-1 букву D, означающую передвижение вниз
и M-1 букву R, означающую передвижение направо. Если таких последовательностей несколько,
необходимо вывести ровно одну (любую) из них.

Пример ввода:
5 5
9 9 9 9 9
3 0 0 0 0
9 9 9 9 9
6 6 6 6 8
9 9 9 9 9

Пример вывода:
74
D D R R R R D D

 */

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <array>

namespace {
    void test0() {
        // int size[2];
        // for (int i{0}; i < 2; ++i) {
        //     std::cin >> size[i];
        // }
        //
        // int map[size[0]][size[1]];
        // for (int r{}; r < size[0]; ++r) {
        //     for (int c{}; c < size[1]; ++c) {
        //         std::cin >> map[r][c];
        //     }
        // }

        int size[] = {5, 5};
        int map[5][5] = {
            {9, 9, 9, 9, 9},
            {3, 0, 0, 0, 0},
            {9, 9, 9, 9, 9},
            {6, 6, 6, 6, 8},
            {9, 9, 9, 9, 9}
        };

        std::vector<uint8_t> allowed_paths;
        for (uint8_t i{0}; i < 0xFF; ++i) {
            uint8_t zero_counter{0};
            for (uint8_t j{0}; j < 8; ++j) {
                if (0x1 & (i >> j)) ++zero_counter;
            }
            if (zero_counter != 4) continue;

            allowed_paths.push_back(i);
        }

        std::map<int, uint8_t> paths_by_count;
        std::priority_queue<int, std::vector<int>, std::less<>> queue;
        for (const auto &path : allowed_paths) {
            int count{map[0][0]};
            int r{0}, c{0};
            for (int i{0}; i < 8; ++i) {
                if (0x1 & (path >> i)) ++r;
                else ++c;
                count += map[r][c];
            }
            queue.push(count);
            paths_by_count[count] = path;
        }

        const auto count{queue.top()};
        std::cout << count << std::endl;
        for (int i{0}; i < 8; ++i) {
            std::cout
                << ((0x1 & (paths_by_count[count] >> i)) ? "D" : "R")
                << " ";
        }
        std::cout << std::endl;
    }

    struct Node {
        std::string path;
        uint16_t value{0};
        Node* down_node{nullptr};
        Node* right_node{nullptr};

        explicit Node(const uint16_t value, std::string path): path {std::move(path)}, value{value} {}

        void print(const std::string& offset) const {
            if (!down_node && !right_node) {
                std::cout << "path: '" << path << "' | " << offset << "value: " << value << std::endl;
            }
            if (down_node) down_node->print(offset + " ");
            if (right_node) right_node->print(offset + " ");
        }

        void collect(std::priority_queue<std::pair<uint16_t, std::string>, std::vector<std::pair<uint16_t, std::string>>, std::less<>>& queue) const {
            if (!down_node && !right_node) {
                queue.emplace(value, path);
            }
            if (down_node) down_node->collect(queue);
            if (right_node) right_node->collect(queue);
        }
    };

    Node* create_node(const std::vector<std::vector<uint16_t>> &map,
                      const int r,
                      const int c,
                      const uint16_t prev_value,
                      const std::string& path) {
        unsigned short current_value{static_cast<unsigned short>(map[r][c] + prev_value)};
        const auto node{new Node{current_value, path}};
        if (r < (map.size() - 1)) {
            node->down_node = create_node(map, r + 1, c, current_value, path + "D ");
        }
        if (c < (map[0].size() - 1)) {
            node->right_node = create_node(map, r, c + 1, current_value, path + "R ");
        }

        return node;
    };

    void delete_node(const Node* node) {
        if (node == nullptr) return;
        delete_node(node->down_node);
        delete_node(node->right_node);

        delete node;
    }

    void test1() {
        int size[2];
        for (int i{0}; i < 2; ++i) {
            std::cin >> size[i];
        }

        uint16_t buffer;
        std::vector<std::vector<uint16_t>> map{};
        for (int r{}; r < size[0]; ++r) {
            map.emplace_back();
            for (int c{}; c < size[1]; ++c) {
                std::cin >> buffer;
                map[r].push_back(buffer);
            }
        }

        // int size[] = {5, 5};
        // std::vector<std::vector<uint16_t>> map {
        //     {9, 9, 9, 9, 9},
        //     {3, 0, 0, 0, 0},
        //     {9, 9, 9, 9, 9},
        //     {6, 6, 6, 6, 8},
        //     {9, 9, 9, 9, 9}
        // };

        const auto node{create_node(map, 0, 0, 0, "")};
        node->print("");

        std::priority_queue<std::pair<uint16_t, std::string>, std::vector<std::pair<uint16_t, std::string>>, std::less<>> queue;
        node->collect(queue);

        const auto& cur{queue.top()};
        std::cout << cur.first << std::endl;
        std::cout << cur.second << std::endl;

        delete_node(node);
    }

    void test2() {
        int size[2];
        for (int i{0}; i < 2; ++i) {
            std::cin >> size[i];
        }

        int buf;
        std::vector<std::vector<int>> map{};
        for (int r{}; r < size[0]; ++r) {
            map.emplace_back();
            for (int c{}; c < size[1]; ++c) {
                std::cin >> buf;
                map[r].push_back(buf);
            }
        }

        // int size[] = {5, 5};
        // std::vector<std::vector<uint16_t>> map {
        //     {9, 9, 9, 9, 9},
        //     {3, 0, 0, 0, 0},
        //     {9, 9, 9, 9, 9},
        //     {6, 6, 6, 6, 8},
        //     {9, 9, 9, 9, 9}
        // };

        const auto& R{size[0]};
        const auto& C{size[1]};

        std::vector<std::vector<int>> buffer(R, std::vector<int>(C));
        std::vector<std::vector<std::string>> paths(R, std::vector<std::string>(C));
        buffer[0][0] = map[0][0];
        paths[0][0] = "";
        for (int r{}; r < R; ++r) {
            for (int c{}; c < C; ++c) {
                if (r == 0 && c == 0) continue;
                int best{INT_MIN};
                std::string hop;
                std::string best_path;
                if (r > 0) {
                    if (best < buffer[r-1][c]) {
                        best = buffer[r-1][c];
                        best_path = paths[r-1][c];
                        hop = "D ";
                    }
                }
                if (c > 0) {
                    if (best < buffer[r][c-1]) {
                        best = buffer[r][c-1];
                        best_path = paths[r][c-1];
                        hop = "R ";
                    }
                }

                buffer[r][c] = best + map[r][c];
                paths[r][c] = best_path + hop;
            }
        }

        std::cout << buffer[R-1][C-1] << std::endl;
        std::cout << paths[R-1][C-1] << std::endl;
    }
}

int main(int argc, char *argv[]) {
    // test0();
    // test1();
    test2();

    return 0;
}
