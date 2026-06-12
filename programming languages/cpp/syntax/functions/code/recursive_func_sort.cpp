#include <iostream>

using std::cout;
using std::endl;

void sort(int[], size_t, size_t);
void swap(int[], size_t, size_t);

int main(int argc, char const *argv[]) {
    int nums[] {3, 0, 6, -2, -6, 11, 3};
    sort(nums, 0, std::size(nums) - 1);
    for (auto num: nums) {
        cout << num << "\t";
    }
    cout << endl;

    return 0;
}

void sort(int numbers[], size_t start, size_t end) {
    if (start >= end) {
        return;
    }

    size_t current {start};
    for (size_t i {start + 1}; i <= end; i++) {
        if (numbers[i] < numbers[start]) {
            swap(numbers, ++current, i);
        }
    }

    swap(numbers, start, current);
    if (current > start) {
        sort(numbers, start, current - 1);
    }
    if (end > current + 1) {
        sort(numbers, current + 1, end);
    }
}

void swap(int numbers[], size_t first, size_t second) {
    auto temp {numbers[first]};
    numbers[first] = numbers[second];
    numbers[second] = temp;
}
