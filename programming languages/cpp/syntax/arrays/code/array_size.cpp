#include <iostream>

using std::size;
using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int nums[4] {1, 2, 3, 4};

    cout << "sizeof(nums) <= " << sizeof(nums) << endl;
    cout << "sizeof(nums[0]) <= " << sizeof(nums[0]) << endl;
    cout << "size(nums) <= " << size(nums) << endl;

    return 0;
}
