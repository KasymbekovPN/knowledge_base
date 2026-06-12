#include <iostream>
#include <algorithm>
#include <memory>

class SArray {
private:
    std::unique_ptr<int[]> data;
    size_t size;

public:
    explicit SArray(size_t _size):
        data{std::make_unique<int[]>(_size)},
        size{_size} {}
    SArray(const SArray& _other):
        data{std::make_unique<int[]>(_other.size)},
        size{_other.size} {
        
        std::copy(
            _other.data.get(),
            _other.data.get() + _other.size,
            data.get()
        );
    }

    SArray(SArray&&) noexcept = default;

    SArray& operator=(SArray _other) noexcept {
        swap(_other);
        return *this;
    }

    void swap(SArray& _other) noexcept {
        std::swap(data, _other.data);
        std::swap(size, _other.size);
    }

    size_t getSize() const {
        return size;
    }
};

int main() {
    auto&& s0 = SArray{7};
    auto&& s1 = SArray{5};
    s1 = s0;
    std::cout
        << "s1 size: "
        << s1.getSize() << std::endl;

    return 0;
}
