/*

https://coderun.yandex.ru/problem/knight-move

Дана прямоугольная доска N×M (N строк и M столбцов). В левом верхнем углу находится шахматный конь, которого необходимо
переместить в правый нижний угол доски. В данной задаче конь может перемещаться на две клетки вниз и одну клетку вправо
или на одну клетку вниз и две клетки вправо.

Необходимо определить, сколько существует различных маршрутов, ведущих из левого верхнего в правый нижний угол.
Путь нулевой длины также считается корректным маршрутом.

Формат ввода
Входной файл содержит два натуральных числа N и M (1⩽N, M⩽50).

Формат вывода
В выходной файл выведите единственное число — количество способов добраться конём до правого нижнего угла доски.

Пример 1:
Ввод:
3 2
Вывод:
1

Пример 2:
Ввод:
31 43
Вывод:
293930

 */

#include <iostream>
#include <vector>

namespace {
    bool both_zero(const int r, const int c) { return r == 0 && c == 0; }

    bool plus_two_one(const int one, const int ONE, const int two, const int TWO ) {
        return one + 1 < ONE && two + 2 < TWO;
    }

    void test() {
        int size[2];
        for (int i{0}; i < 2; ++i) {
            std::cin >> size[i];
        }
        // int size[2] {3, 2};
        // int size[2] {31, 43};
        // int size[] {9, 5};

        const auto& R{size[0]};
        const auto& C{size[1]};

        std::vector<std::vector<int>> buffer(R, std::vector<int>(C, 0));
        buffer[0][0] = 1;
        for (int r{}; r < R; ++r) {
            for (int c{}; c < C; ++c) {
                if ((both_zero(r, c) || buffer[r][c]) && plus_two_one(c, C, r, R)) {
                    buffer[r+2][c+1] += buffer[r][c];
                }
                if ((both_zero(r, c) || buffer[r][c]) && plus_two_one(r, R, c, C)) {
                    buffer[r+1][c+2] += buffer[r][c];
                }
            }
        }

        std::cout << buffer[R-1][C-1] << std::endl;
    }
}

int main() {
    test();

    return 0;
}

