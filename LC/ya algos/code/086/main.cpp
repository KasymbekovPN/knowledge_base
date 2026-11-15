#include <iostream>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <random>

namespace {

// An array of `length` integers, all zero at the start.
// snap() takes a snapshot and returns its id: 0, 1, 2, ... in order.
// get(index, snap_id) returns the value that index had when snapshot snap_id was taken.
// get is called only with ids of snapshots that were already taken.
class SnapshotArray {
	struct Record {
		int value;
		int snap_id;
	};
	std::vector<std::vector<Record>> storage_;
	int next_snap_id_{0};

public:
    explicit SnapshotArray(const int capacity):
	    storage_(static_cast<std::size_t>(capacity)) {}

    void set(const int idx, const int value) {
	    auto& records{storage_[idx]};

	    if (!records.empty() && records.back().snap_id == next_snap_id_)  {
		    records.back().value = value;
	    } else {
		    records.push_back({value, next_snap_id_});
	    }
    }

    int snap() { return next_snap_id_++; }

    int get(const int idx, const int snap_id) const {
	    auto& records{storage_[idx]};

	    const auto it{std::upper_bound(
		    records.begin(),
		    records.end(),
		    snap_id,
		    [](const int id, const Record& record) {
			    return id < record.snap_id;
		    }
	    )};

	    return it != records.begin() ? std::prev(it)->value : 0;
    }
};

struct Op {
    char kind;  // 'S' set, 'P' snap, 'G' get
    int a;      // index
    int b;      // value for set, snapshot id for get
};

Op S(int index, int value) { return {'S', index, value}; }
Op P() { return {'P', 0, 0}; }
Op G(int index, int snap_id) { return {'G', index, snap_id}; }

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

// Reference for small scripts: a full copy of the array for every snapshot.
std::vector<int> expected_naive(int length, const std::vector<Op>& ops) {
    std::vector<int> current(static_cast<std::size_t>(length), 0);
    std::vector<std::vector<int>> snapshots;
    std::vector<int> answers(ops.size(), 0);
    for (std::size_t i = 0; i < ops.size(); ++i) {
        switch (ops[i].kind) {
            case 'S': current[static_cast<std::size_t>(ops[i].a)] = ops[i].b; break;
            case 'P': snapshots.push_back(current); answers[i] = static_cast<int>(snapshots.size()) - 1; break;
            default: answers[i] = snapshots[static_cast<std::size_t>(ops[i].b)][static_cast<std::size_t>(ops[i].a)]; break;
        }
    }
    return answers;
}

// Reference for big tests: for every touched index a map "snapshot id -> value" (last set wins).
std::vector<int> expected_history(int length, const std::vector<Op>& ops) {
    std::vector<std::map<int, int>> history(static_cast<std::size_t>(length));
    std::vector<int> answers(ops.size(), 0);
    int next_snap = 0;
    for (std::size_t i = 0; i < ops.size(); ++i) {
        switch (ops[i].kind) {
            case 'S': history[static_cast<std::size_t>(ops[i].a)][next_snap] = ops[i].b; break;
            case 'P': answers[i] = next_snap++; break;
            default: {
                const auto& h = history[static_cast<std::size_t>(ops[i].a)];
                const auto it = h.upper_bound(ops[i].b);
                answers[i] = (it == h.begin()) ? 0 : std::prev(it)->second;
                break;
            }
        }
    }
    return answers;
}

int apply(SnapshotArray& arr, const Op& op) {
    switch (op.kind) {
        case 'S': arr.set(op.a, op.b); return 0;
        case 'P': return arr.snap();
        default: return arr.get(op.a, op.b);
    }
}

void test_script(const std::string& label, int length, const std::vector<Op>& ops) {
    const std::vector<int> expected = expected_naive(length, ops);
    SnapshotArray arr(length);
    bool ok = true;
    for (std::size_t i = 0; i < ops.size(); ++i) {
        const int got = apply(arr, ops[i]);
        if (ops[i].kind != 'S' && got != expected[i]) ok = false;
    }
    report(label, ok);
}

void test_big(const std::string& label, int length, const std::vector<Op>& ops) {
    std::vector<int> answers(ops.size(), 0);
    const auto start = std::chrono::steady_clock::now();
    {
        SnapshotArray arr(length);
        for (std::size_t i = 0; i < ops.size(); ++i) answers[i] = apply(arr, ops[i]);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const std::vector<int> expected = expected_history(length, ops);
    bool correct = true;
    for (std::size_t i = 0; i < ops.size() && correct; ++i) {
        if (ops[i].kind != 'S') correct = answers[i] == expected[i];
    }
    const bool fast = elapsed < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), correct && fast);
}

// Random operations: sets, snaps and gets of already taken snapshots.
std::vector<Op> random_ops(std::uint32_t seed, int length, int count, int set_permille, int snap_permille) {
    std::mt19937 gen(seed);
    std::vector<Op> ops;
    int taken = 0;
    for (int i = 0; i < count; ++i) {
        const int r = static_cast<int>(gen() % 1000);
        if (r < set_permille) {
            ops.push_back(S(static_cast<int>(gen() % static_cast<unsigned>(length)), static_cast<int>(gen())));
        } else if (r < set_permille + snap_permille || taken == 0) {
            ops.push_back(P());
            ++taken;
        } else {
            ops.push_back(G(static_cast<int>(gen() % static_cast<unsigned>(length)),
                            static_cast<int>(gen() % static_cast<unsigned>(taken))));
        }
    }
    return ops;
}

void test_all() {
	test_script("description example", 3,
		{S(0, 5), P(), S(0, 6), G(0, 0)});
	test_script("initial values are zero", 4,
		{P(), G(0, 0), G(3, 0), S(1, 9), P(), G(1, 0), G(1, 1), G(2, 1)});
	test_script("snap ids are consecutive", 2,
		{P(), P(), P(), S(0, 1), P(), P()});
	test_script("a snapshot does not see later sets", 2,
		{S(0, 1), P(), S(0, 2), P(), S(0, 3), P(), G(0, 0), G(0, 1), G(0, 2)});
	test_script("get of an older snapshot after many changes", 2,
		{S(1, 10), P(), S(1, 20), P(), S(1, 30), P(), S(1, 40), P(), G(1, 0), G(1, 1), G(1, 2), G(1, 3)});
	test_script("several sets of one index between snapshots: the last one wins", 2,
		{S(0, 1), S(0, 2), S(0, 3), P(), S(0, 4), S(0, 5), P(), G(0, 0), G(0, 1)});
	test_script("an index that was never set returns zero in every snapshot", 3,
		{S(0, 7), P(), P(), P(), G(2, 0), G(2, 1), G(2, 2), G(1, 2)});
	test_script("an index first set after several snapshots", 2,
		{P(), P(), P(), S(1, 5), P(), G(1, 0), G(1, 1), G(1, 2), G(1, 3)});
	test_script("setting the same value again", 2,
		{S(0, 5), P(), S(0, 5), P(), S(0, 6), P(), G(0, 0), G(0, 1), G(0, 2)});
	test_script("setting back to zero", 2,
		{S(0, 5), P(), S(0, 0), P(), G(0, 0), G(0, 1)});
	test_script("length 1", 1,
		{P(), S(0, 3), P(), S(0, 4), P(), G(0, 0), G(0, 1), G(0, 2)});
	test_script("negative and extreme values", 3,
		{S(0, -1), S(1, 2147483647), S(2, -2147483648), P(), S(0, 2147483647), S(1, -2147483648), P(), G(0, 0), G(1, 0), G(2, 0), G(0, 1), G(1, 1), G(2, 1)});
	test_script("the first and the last index", 5,
		{S(0, 1), S(4, 2), P(), S(0, 3), S(4, 4), P(), G(0, 0), G(4, 0), G(0, 1), G(4, 1)});
	test_script("get right after snap", 3,
		{S(0, 1), P(), G(0, 0), S(0, 2), P(), G(0, 1), G(0, 0)});
	test_script("independent indices", 3,
		{S(0, 1), P(), S(1, 2), P(), S(2, 3), P(), G(0, 0), G(1, 0), G(2, 0), G(0, 2), G(1, 2), G(2, 2)});
	test_script("get of the same snapshot many times", 2,
		{S(0, 9), P(), S(0, 10), P(), G(0, 0), G(0, 0), G(0, 0), G(0, 1), G(0, 1), G(0, 1)});
	test_script("random 1: length 2, 30 operations", 2,
		{P(), G(0, 0), S(1, -19), G(1, 0), S(1, -18), S(1, -4), G(0, 0), S(0, -17), G(1, 0), G(0, 0), S(1, 11), G(0, 0), G(1, 0), S(0, 9), S(0, 14), P(), S(1, 18), S(0, 9), P(), S(1, 8), S(0, -1), P(), S(1, 12), G(1, 0), S(1, 12), G(0, 2), G(0, 3), G(0, 3), G(0, 0), P()});
	test_script("random 2: length 3, 60 operations", 3,
		{P(), S(0, -20), S(0, 11), G(0, 0), G(1, 0), S(1, -13), G(2, 0), S(0, -4), G(1, 0), S(0, 13), G(1, 0), G(2, 0), G(0, 0), S(1, 18), S(0, 5), S(1, 0), S(1, 13), S(2, 11), P(), G(0, 1), S(2, -20), P(), S(1, 2), S(1, 3), G(2, 2), G(1, 1), S(2, 14), S(1, -13), P(), S(1, -19), S(1, 3), S(1, 15), S(2, -5), G(0, 2), S(1, -8), S(0, -12), G(0, 0), S(2, -12), S(0, -5), G(1, 0), S(2, 20), S(0, 13), G(2, 2), S(0, 11), P(), S(0, 14), G(1, 1), G(0, 1), S(2, -4), S(0, -9), G(2, 4), P(), G(1, 4), P(), G(1, 1), S(0, 9), S(0, 15), G(0, 3), S(0, 2), G(1, 0)});
	test_script("random 3: length 5, 120 operations", 5,
		{P(), S(2, 11), G(0, 0), S(2, 14), G(0, 0), G(3, 0), G(0, 0), G(4, 0), S(4, 13), G(4, 0), P(), P(), S(0, -5), G(3, 1), S(3, -2), S(4, 11), S(3, 15), P(), G(0, 0), S(3, -5), G(4, 2), S(3, 19), S(2, 13), S(4, 11), S(4, 0), P(), S(3, -11), G(4, 1), S(3, 16), S(1, 14), S(0, 7), S(4, -17), S(1, -18), G(2, 0), S(3, 11), S(4, 6), S(0, 15), G(2, 3), G(1, 0), S(4, 18), P(), P(), S(1, -2), S(2, 18), G(3, 0), P(), G(3, 2), P(), S(4, 19), G(3, 0), G(1, 2), G(4, 4), G(2, 8), G(1, 4), G(2, 6), P(), S(3, 11), S(4, 7), S(0, -20), S(0, 6), P(), G(1, 9), G(1, 2), G(0, 6), G(3, 6), S(2, 6), G(3, 0), P(), G(1, 0), S(0, -18), S(2, 18), S(3, -8), G(2, 2), G(2, 7), G(2, 2), P(), G(1, 4), G(4, 5), P(), S(2, -20), G(3, 2), G(0, 13), S(4, 17), S(3, 14), G(4, 4), S(4, 18), S(4, -18), S(3, -19), S(0, 15), G(3, 10), S(4, 3), S(3, -7), G(0, 12), S(3, -5), G(0, 2), G(3, 9), G(0, 10), G(4, 13), S(2, -7), G(4, 8), G(4, 2), G(1, 11), S(0, -20), G(0, 0), G(4, 11), G(3, 7), S(3, 2), G(0, 6), S(3, 1), G(4, 11), S(3, 10), G(0, 7), S(3, -7), S(4, 10), S(1, -9), P(), P(), S(3, -3), S(2, -10), P()});
	test_script("random 4: length 8, 300 operations", 8,
		{S(0, -7), P(), G(2, 0), S(3, 18), G(1, 0), G(5, 0), G(6, 0), P(), G(3, 1), G(0, 0), S(6, 3), G(3, 0), G(5, 0), G(5, 1), G(4, 0), P(), G(4, 1), S(7, -19), P(), P(), S(6, 14), S(3, -8), G(2, 4), S(5, -18), S(1, -13), P(), P(), S(5, -12), P(), S(4, 16), P(), G(0, 8), P(), S(7, 0), S(0, -12), G(5, 9), S(7, -19), S(2, -17), S(7, 5), G(3, 7), S(5, 16), G(4, 5), G(7, 1), G(4, 4), S(7, 0), G(0, 3), P(), P(), G(6, 11), G(5, 0), G(3, 2), P(), S(7, -5), G(5, 8), S(0, 0), S(1, -3), G(2, 11), S(1, 15), G(7, 10), G(5, 1), G(3, 7), G(0, 8), G(4, 3), P(), G(3, 3), G(5, 9), S(7, 10), G(2, 2), P(), S(3, -7), G(6, 9), G(4, 10), P(), P(), S(6, -7), G(0, 2), P(), P(), S(0, -18), S(0, -12), S(2, 1), S(5, 12), G(6, 0), S(0, 3), G(7, 5), G(7, 9), G(0, 5), G(3, 14), S(4, 16), S(3, 3), P(), S(2, -19), G(6, 12), S(5, -1), S(2, 3), S(0, 0), G(3, 1), P(), G(7, 16), P(), P(), S(5, -7), S(0, 20), P(), G(3, 16), S(2, 20), S(6, 13), G(0, 3), S(3, 7), G(4, 2), S(0, -7), S(3, -14), G(2, 9), S(5, -20), P(), G(1, 8), S(5, 5), P(), G(4, 18), G(5, 2), G(1, 5), S(5, -5), S(3, 5), G(5, 3), S(5, 9), G(6, 20), G(5, 24), G(3, 16), G(5, 0), G(2, 4), G(5, 0), S(7, 13), G(0, 23), S(5, 15), S(6, 10), G(4, 19), P(), S(7, 16), S(6, 5), S(6, 18), P(), S(2, -5), S(3, -16), G(0, 21), S(7, -17), S(7, 16), G(7, 7), G(6, 12), G(1, 24), S(0, -16), G(5, 25), S(1, -9), G(2, 21), S(2, -5), S(4, 19), G(2, 16), S(7, 2), S(6, 1), S(2, -17), S(1, 4), S(0, -12), G(2, 2), S(5, -18), S(6, 10), G(6, 22), G(3, 2), S(5, 19), S(7, 16), S(5, 18), G(3, 2), G(0, 16), S(3, -3), P(), S(3, 4), S(0, 15), S(1, 2), S(5, -13), G(5, 11), S(4, -12), S(1, 11), S(5, -17), S(7, 12), S(1, 1), G(2, 4), S(4, -7), S(3, 8), G(5, 28), P(), P(), S(5, -19), G(2, 15), G(6, 10), S(7, -15), S(3, 0), G(3, 4), G(7, 17), G(7, 19), S(3, 19), S(6, -2), S(5, -3), S(1, -11), P(), S(1, -2), G(7, 16), P(), S(0, 15), G(7, 30), S(2, -13), G(6, 5), S(2, -13), G(7, 10), P(), G(0, 14), G(7, 24), S(1, -8), S(1, -14), S(0, -15), S(2, 11), P(), S(7, 15), S(7, -8), S(0, 16), P(), S(6, 3), S(7, 11), G(4, 15), S(4, 8), S(7, -19), G(6, 24), P(), G(7, 14), S(3, -2), G(6, 22), S(5, -17), S(4, 5), P(), S(2, 4), G(7, 24), G(7, 10), S(2, -20), S(5, -4), G(7, 9), P(), P(), G(6, 1), P(), G(7, 1), S(3, -10), S(2, -18), S(3, -9), G(3, 2), G(7, 40), G(3, 4), G(0, 35), S(4, 14), P(), S(4, 6), G(5, 1), G(2, 15), P(), G(7, 4), S(3, 3), G(0, 5), G(7, 4), S(4, -9), S(7, -15), G(5, 9), G(2, 30), G(6, 5), S(0, -19), P(), S(1, -12), G(5, 25), S(0, 13), G(0, 32), S(6, -17), S(6, -10), S(1, -6), S(1, 4), S(4, 11), G(3, 32), S(6, -17), P(), G(2, 4), S(2, 9), S(0, 12), S(7, 20), S(2, 14), G(5, 2), G(7, 23), S(0, 13), S(4, 10), S(3, 2), S(1, 16), S(6, -7), S(7, -11), G(6, 20), G(0, 8), S(1, -5), P()});
	test_script("random 5: length 4, 300 operations", 4,
		{P(), G(1, 0), S(1, 19), S(0, 15), G(3, 0), G(1, 0), G(1, 0), S(0, -3), S(2, 2), G(1, 0), G(3, 0), S(0, 14), G(2, 0), S(0, 19), P(), S(1, 10), P(), G(1, 2), S(3, -10), S(2, 4), G(3, 1), S(0, 7), G(0, 0), S(1, 20), S(3, 12), G(0, 0), S(1, 20), S(3, -12), G(1, 2), G(2, 1), G(0, 2), S(1, 2), S(2, -9), P(), S(0, 1), G(3, 1), P(), S(1, 17), G(2, 2), G(1, 2), S(2, -15), P(), G(2, 0), S(0, -3), G(2, 0), S(2, 10), G(1, 0), S(3, 1), S(3, 13), S(3, -11), G(1, 4), S(0, 11), P(), G(0, 5), S(2, -15), G(1, 4), S(0, 8), G(0, 3), S(2, 17), G(0, 2), G(0, 0), S(1, 0), G(1, 4), P(), G(1, 1), P(), S(1, 11), S(3, -5), S(2, 11), S(0, 2), G(3, 0), P(), P(), G(2, 4), S(1, 12), S(0, -13), S(3, -8), S(0, 3), S(3, -3), S(0, 10), G(1, 3), S(3, 15), S(0, 17), G(0, 10), G(1, 1), P(), G(2, 7), P(), S(2, -10), S(3, -4), P(), S(2, 4), S(2, 7), S(3, -17), S(2, -2), G(1, 9), S(2, -18), P(), S(3, 1), G(2, 8), G(2, 10), G(3, 2), P(), S(3, -20), G(1, 3), S(3, -5), P(), G(1, 0), G(0, 4), G(2, 1), G(2, 4), S(1, 15), S(3, 0), G(2, 2), S(2, 13), S(1, 15), P(), S(2, -2), G(0, 7), S(3, 8), P(), P(), S(3, -14), G(1, 6), S(1, -6), S(1, 6), G(0, 1), S(1, 13), G(2, 2), P(), P(), S(3, -1), G(2, 1), G(0, 4), S(3, 3), G(3, 5), S(0, -20), G(1, 19), S(1, 6), P(), G(0, 0), P(), G(0, 5), G(0, 2), G(2, 16), S(2, 15), S(2, 1), S(3, -10), G(2, 11), S(0, 14), G(0, 15), S(1, -14), G(0, 5), P(), G(3, 10), G(1, 0), S(0, 17), S(0, -6), G(0, 22), S(1, 12), S(2, -14), G(3, 19), G(1, 11), S(3, 19), S(3, -1), S(3, -5), G(3, 19), S(1, -7), S(1, -18), S(2, 20), G(3, 21), S(0, -8), G(2, 24), S(2, -5), G(0, 16), S(1, -10), S(1, -9), G(1, 22), S(3, 8), S(2, -7), G(2, 14), S(0, 6), S(3, 20), G(3, 17), P(), P(), S(1, -19), G(1, 10), G(1, 10), S(3, -19), S(0, -12), S(3, -18), G(2, 17), P(), G(1, 2), G(0, 20), G(0, 15), S(0, 17), G(3, 8), G(3, 8), S(2, -13), S(3, -11), S(0, -13), S(1, -12), G(2, 20), S(0, -7), S(0, -6), G(1, 6), P(), S(2, -11), G(3, 27), S(0, 2), G(2, 28), S(2, 16), G(1, 20), S(2, 5), G(0, 21), G(0, 10), S(1, 5), G(2, 9), G(1, 12), S(3, -20), S(2, -12), S(2, -17), G(1, 27), G(3, 23), G(0, 27), S(0, 18), S(1, 0), G(1, 7), S(3, 16), G(1, 17), S(1, 18), S(2, 17), S(3, 8), G(0, 28), G(0, 26), G(3, 6), G(3, 10), G(2, 26), S(3, 16), P(), G(1, 27), G(2, 28), S(2, -3), S(2, 20), S(0, -12), S(0, 5), G(3, 26), G(0, 3), G(1, 1), G(1, 28), G(1, 22), G(2, 5), G(3, 5), P(), S(3, -17), G(0, 23), G(1, 9), S(2, 7), G(3, 25), G(1, 24), P(), S(2, -1), G(0, 18), S(2, -6), G(2, 9), S(1, 7), S(1, 18), S(2, 3), S(2, 17), G(0, 21), S(2, -11), G(1, 20), S(3, 8), S(3, -4), S(0, 13), P(), S(3, 10), G(3, 3), S(1, 1), S(0, 13), S(2, 11), G(3, 27), G(3, 31), G(3, 19), S(0, 3), G(0, 1), S(2, 12), P(), S(3, -4), S(3, 5), P(), G(1, 27), P(), S(2, -13), G(3, 17), P(), G(0, 33), P()});
	test_script("random 6: length 20, 300 operations", 20,
		{P(), G(17, 0), S(8, -1), P(), S(3, 1), S(9, 16), S(16, 2), P(), S(9, -1), G(0, 1), S(15, 3), S(12, 2), S(16, 14), S(12, -12), G(3, 1), G(9, 0), G(19, 2), G(19, 1), G(10, 1), P(), P(), P(), G(11, 0), G(12, 2), S(12, 10), S(12, 18), P(), S(19, 16), P(), P(), S(3, 17), S(19, 1), G(4, 0), G(8, 3), P(), S(11, -16), G(9, 8), G(2, 5), G(18, 5), S(4, 18), S(17, 2), S(7, -6), G(5, 4), S(5, 20), G(18, 5), S(18, 2), S(12, 18), S(18, -16), S(17, 3), G(18, 9), P(), S(5, 14), P(), G(7, 2), S(2, 3), S(15, 2), G(6, 6), P(), P(), S(1, 18), S(0, 1), S(13, 4), S(11, -20), G(19, 1), G(13, 1), S(13, -1), S(9, -14), P(), P(), G(12, 3), S(1, -4), G(19, 14), S(7, -8), P(), P(), G(15, 8), G(1, 3), G(3, 6), G(3, 5), P(), S(17, 12), G(3, 8), S(9, 2), G(7, 13), P(), G(19, 18), S(9, -13), G(3, 14), G(19, 8), G(1, 17), S(1, 19), S(9, -6), S(0, 7), S(16, 1), P(), G(8, 13), S(13, -16), S(19, 7), P(), G(13, 15), S(17, 16), G(19, 14), P(), S(16, -6), P(), G(12, 2), S(2, -11), P(), S(8, -15), G(3, 2), S(13, -9), S(18, 10), G(17, 6), S(8, 15), S(5, -13), S(13, -15), G(13, 6), S(5, 8), S(14, 13), S(5, -19), G(0, 4), S(6, 7), P(), S(8, 15), G(11, 2), P(), S(17, 9), P(), S(6, 2), G(16, 8), S(17, -17), G(14, 11), G(6, 14), G(8, 26), G(2, 13), S(2, 12), S(9, -9), S(3, 15), P(), G(2, 25), S(16, 13), P(), G(9, 16), P(), S(19, -6), S(17, -11), P(), S(11, 10), S(10, 12), S(17, -12), G(11, 17), G(7, 18), G(11, 19), S(17, -15), G(12, 27), G(1, 9), P(), G(19, 29), S(17, 1), P(), S(11, -11), S(3, 7), S(5, -10), P(), P(), S(3, 11), G(6, 23), G(16, 23), S(0, -2), S(13, 16), S(19, -10), P(), P(), G(1, 16), G(17, 1), G(4, 8), P(), G(17, 5), S(17, -19), S(6, 14), P(), G(13, 20), G(6, 28), G(5, 28), S(14, 16), G(8, 24), G(11, 7), G(14, 38), G(8, 15), G(7, 39), S(19, -2), G(14, 22), P(), S(9, -5), G(15, 36), G(8, 18), G(3, 26), S(14, 11), G(13, 6), S(8, 15), S(7, -19), P(), S(19, 4), G(7, 8), G(9, 24), S(17, -12), S(15, -17), S(19, 17), S(17, -8), G(15, 9), S(1, -13), S(8, -14), G(14, 33), S(19, -20), S(12, 5), G(19, 2), G(18, 38), S(8, -18), S(4, -2), S(17, -7), G(10, 5), S(8, -5), S(16, -10), S(4, -6), G(17, 25), S(19, 5), G(5, 5), P(), G(2, 36), P(), S(6, -19), S(14, -3), G(14, 37), S(7, -12), P(), S(19, -10), P(), S(18, 1), S(7, -20), G(9, 11), S(2, -18), S(16, -19), G(3, 39), S(15, -9), G(13, 3), S(14, -8), P(), G(2, 2), S(1, -1), S(19, -8), S(9, 17), S(5, -13), S(17, 1), G(9, 6), P(), S(12, 20), G(6, 44), G(9, 8), S(8, 1), S(14, 8), P(), S(6, -8), S(2, -18), S(3, 14), S(10, 8), G(14, 27), G(18, 1), S(8, 13), S(16, 11), G(17, 12), G(0, 24), P(), G(17, 17), S(0, 6), S(16, -3), S(11, -8), G(1, 49), G(19, 23), S(5, 15), G(7, 11), S(19, -13), S(6, -11), S(4, 10), S(4, 10), S(5, 4), G(19, 19), G(3, 20), S(8, 9), S(19, 9), S(15, 13), G(18, 41), S(3, -10), G(7, 18), P(), G(7, 22), S(4, -10), G(0, 38), S(9, -6), S(19, 12), G(3, 10)});
	test_script("random 7: length 1, 100 operations", 1,
		{P(), S(0, 1), P(), G(0, 1), S(0, -4), G(0, 1), G(0, 1), S(0, 15), G(0, 1), P(), G(0, 1), P(), G(0, 3), P(), P(), P(), G(0, 1), P(), G(0, 1), G(0, 2), G(0, 3), S(0, -20), P(), S(0, 12), S(0, -2), S(0, 16), G(0, 5), G(0, 8), S(0, -8), P(), S(0, 0), S(0, -20), P(), S(0, -4), P(), S(0, -13), S(0, -7), S(0, 6), S(0, -5), S(0, -6), P(), S(0, -5), S(0, 16), S(0, 6), G(0, 12), G(0, 6), S(0, -19), S(0, 13), S(0, 3), S(0, -2), G(0, 10), G(0, 7), S(0, 18), P(), S(0, 4), G(0, 7), S(0, -4), S(0, 18), G(0, 9), S(0, -12), S(0, 7), S(0, 15), S(0, 10), S(0, 16), S(0, 12), G(0, 11), G(0, 7), S(0, -15), G(0, 11), G(0, 13), S(0, 11), G(0, 6), S(0, 14), S(0, -12), S(0, -4), P(), G(0, 0), S(0, -2), G(0, 6), S(0, -11), P(), S(0, 8), G(0, 7), S(0, 4), G(0, 5), S(0, 16), S(0, 2), S(0, 19), G(0, 3), S(0, -9), S(0, -18), P(), S(0, -18), G(0, 11), S(0, -12), S(0, -13), S(0, 4), P(), P(), P()});

	// ---- big tests ----
	test_big("big: length 100000, 1000000 random operations, about 2000 snapshots", 100000,
		random_ops(1, 100000, 1000000, 600, 2));
	test_big("big: length 20, 1000000 operations, long history per index", 20,
		random_ops(2, 20, 1000000, 400, 100));
	{
		// many snapshots of a big array with almost no changes: copying the array every time is too heavy
		std::vector<Op> ops;
		std::mt19937 gen(3);
		for (int i = 0; i < 30000; ++i) {
			ops.push_back(S(static_cast<int>(gen() % 30000), i + 1));
			ops.push_back(P());
		}
		for (int i = 0; i < 200000; ++i)
			ops.push_back(G(static_cast<int>(gen() % 30000), static_cast<int>(gen() % 30000)));
		test_big("big: length 30000, 30000 snapshots, 200000 gets", 30000, ops);
	}
	{
		// one index changed before every snapshot, queried at every snapshot
		std::vector<Op> ops;
		for (int i = 0; i < 100000; ++i) { ops.push_back(S(0, i)); ops.push_back(P()); }
		for (int i = 0; i < 100000; ++i) ops.push_back(G(0, i));
		test_big("big: one index, 100000 snapshots, then 100000 gets", 1, ops);
	}
}

}  // namespace

int main() {
	test_all();
	return 0;
}