#include <atomic>

namespace {
    std::atomic<long> counter{0};

    void inc_relaxed() { counter.fetch_add(1, std::memory_order_relaxed); }
    long load_acquire() { return counter.load(std::memory_order_acquire); }
    void store_release(const long v) { counter.store(v, std::memory_order_release); }
    void inc_acq_rel() { counter.fetch_add(1, std::memory_order_acq_rel); }
    void inc_seq_cst() { counter.fetch_add(1, std::memory_order_seq_cst); }
    void store_seq_cst(const long v) { counter.store(v, std::memory_order_seq_cst); }

}

int main(int argc, char *argv[]) {
    inc_relaxed();
    load_acquire();
    store_release(21L);
    inc_acq_rel();
    inc_seq_cst();
    store_seq_cst(42L);

    return 0;
}
