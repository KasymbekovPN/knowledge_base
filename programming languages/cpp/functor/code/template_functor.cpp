#include <iostream>

template<typename T>
class Equal {

private:
    T threshold;

public:
    Equal(T threshold): threshold(threshold) {}
    bool operator()(T value) {
        return threshold == value;
    }
};

template<typename T>
void _test(Equal<T>&, const size_t);

int main() {
    const size_t THRESHOLD {42};
    Equal<size_t> functor {THRESHOLD};
    _test<size_t>(functor, THRESHOLD);
    _test<size_t>(functor, 12);

    return 0;
}

template<typename T>
void _test(Equal<T>& functor, const size_t value) {
    std::cout
        << value << " "
        << std::boolalpha
        << functor(value)
        << std::noboolalpha
        << std::endl;
}
