#include <iostream>
#include <chrono>
#include <vector>

using namespace std::chrono;
using std::cout;
using std::endl;
using std::string;

std::vector<int> _get_temp_vector_nrvo(bool, const size_t);
std::vector<int> _get_temp_vector_move(bool, const size_t);
void _print_duration(steady_clock::time_point&, steady_clock::time_point&, string);

int main() {
    auto start_nrvo_1m = high_resolution_clock::now();
    volatile auto vec_nrvo_1m = _get_temp_vector_nrvo(true, 1'000'00);
    auto end_nrvo_1m = high_resolution_clock::now();

    auto start_move_1m = high_resolution_clock::now();
    volatile auto vec_move_1m = _get_temp_vector_move(true, 1'000'00);
    auto end_move_1m = high_resolution_clock::now();

    auto start_nrvo_10m = high_resolution_clock::now();
    volatile auto vec_nrvo_10m = _get_temp_vector_nrvo(true, 10'000'00);
    auto end_nrvo_10m = high_resolution_clock::now();

    auto start_move_10m = high_resolution_clock::now();
    volatile auto vec_move_10m = _get_temp_vector_move(true, 10'000'00);
    auto end_move_10m = high_resolution_clock::now();

    _print_duration(start_nrvo_1m, end_nrvo_1m, "NRVO 1M");
    _print_duration(start_move_1m, end_move_1m, "MOVE 1M");
    _print_duration(start_nrvo_10m, end_nrvo_10m, "NRVO 10M");
    _print_duration(start_move_10m, end_move_10m, "MOVE 10M");

    return 0;
}

std::vector<int> _get_temp_vector_nrvo(bool first, const size_t size) {
    std::vector<int> first_vector(size);
    std::vector<int> second_vector(size);

    // NRVO
    if (first) {
        return first_vector;
    } else {
        return second_vector;
    }
}

std::vector<int> _get_temp_vector_move(bool first, const size_t size) {
    std::vector<int> first_vector(size);
    std::vector<int> second_vector(size);

    if (first) {
        return std::move(first_vector);
    } else {
        return std::move(second_vector);
    }
}

void _print_duration(steady_clock::time_point &start,
                     steady_clock::time_point &end,
                     string label) {
    auto delta = end - start;
    std::cout
        << "[" << label <<"] Time: "
        << duration_cast<std::chrono::nanoseconds>(delta).count()
        << " ns" << std::endl;
}
