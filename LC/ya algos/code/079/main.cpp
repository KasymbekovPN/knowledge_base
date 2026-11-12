#include <iostream>
#include <set>
#include <utility>
#include <unordered_map>
#include <cstddef>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <random>

namespace {

// A tiny search index. Documents are added with strictly increasing ids.
// A document is a list of terms; a term may be repeated inside one document.
// search_and returns the ids of all documents that contain EVERY term of the query,
// in ascending order. The query may repeat terms. An empty query returns nothing,
// a term that is not in the index makes the result empty.
class InvertedIndex {
	static constexpr int BIN_SEARCH_THRESHOLD{32};

	std::unordered_map<std::string, std::vector<int>> storage_{};

public:
    void add_document(int doc_id, const std::vector<std::string>& terms) {
	    for (const auto& term: terms) {
		    auto& by_doc_id{storage_[term]};
		    if (by_doc_id.empty() || by_doc_id.back() != doc_id) {
			    by_doc_id.push_back(doc_id);
		    }
	    }
    }

    std::vector<int> search_and(const std::vector<std::string>& terms) const {
	    if (terms.empty()) return {};

	    std::vector<const std::vector<int>*> according;
	    for (const auto& term: terms) {
		    const auto it{storage_.find(term)};
		    if (it == storage_.end()) return {};

		    according.push_back(&it->second);
	    }

    	std::ranges::sort(according, [](const auto* lhs, const auto* rhs) {
			return lhs->size() < rhs->size();
		});
    	according.erase(std::ranges::unique(according).begin(), according.end());

	    std::vector<int> result{*according[0]};
	    for (std::size_t i{1}; i < according.size() && !result.empty(); ++i) {
		    const auto& other{*according[i]};
		    std::vector<int> next;
		    if (other.size() > static_cast<std::size_t>(BIN_SEARCH_THRESHOLD) * result.size()) {
			    for (const auto id: result) {
			    if (std::ranges::binary_search(other, id))
				    next.push_back(id);
			    }
		    } else {
			    std::ranges::set_intersection(result, other, std::back_inserter(next));
		    }

		    result = std::move(next);
	    }

	    return result;
	}
};

struct Op {
    bool is_add;
    int doc_id;
    std::vector<std::string> terms;
};

Op D(int doc_id, const std::vector<std::string>& terms) { return {true, doc_id, terms}; }
Op Q(const std::vector<std::string>& terms) { return {false, 0, terms}; }

void report(const std::string& label, bool ok) {
    std::cout << label << ": " << (ok ? "OK" : "FAIL") << '\n';
}

// Reference for small scripts: remember every document as a set and scan all of them for a query.
class NaiveModel {
public:
    void add(int doc_id, const std::vector<std::string>& terms) {
        docs_.push_back({doc_id, std::set<std::string>(terms.begin(), terms.end())});
    }
    std::vector<int> search(const std::vector<std::string>& query) const {
        std::vector<int> result;
        if (query.empty()) return result;
        for (const auto& doc : docs_) {
            bool all = true;
            for (const auto& term : query) all = all && doc.second.count(term) > 0;
            if (all) result.push_back(doc.first);
        }
        return result;
    }

private:
    std::vector<std::pair<int, std::set<std::string>>> docs_;
};

// Reference for big tests: sorted list of document ids per term; the query is answered by
// taking the shortest list and checking every id with a binary search in the other lists.
class ListModel {
public:
    void add(int doc_id, const std::vector<std::string>& terms) {
        const std::set<std::string> unique(terms.begin(), terms.end());
        for (const auto& term : unique) lists_[term].push_back(doc_id);
    }
    std::vector<int> search(const std::vector<std::string>& query) const {
        std::vector<int> result;
        if (query.empty()) return result;
        std::vector<const std::vector<int>*> lists;
        for (const auto& term : query) {
            const auto it = lists_.find(term);
            if (it == lists_.end()) return result;
            lists.push_back(&it->second);
        }
        std::sort(lists.begin(), lists.end(),
                  [](const std::vector<int>* a, const std::vector<int>* b) { return a->size() < b->size(); });
        for (int id : *lists.front()) {
            bool all = true;
            for (std::size_t i = 1; i < lists.size() && all; ++i)
                all = std::binary_search(lists[i]->begin(), lists[i]->end(), id);
            if (all) result.push_back(id);
        }
        return result;
    }

private:
    std::unordered_map<std::string, std::vector<int>> lists_;
};

void test_script(const std::string& label, const std::vector<Op>& ops) {
    InvertedIndex index;
    NaiveModel model;
    bool ok = true;
    for (const Op& op : ops) {
        if (op.is_add) {
            index.add_document(op.doc_id, op.terms);
            model.add(op.doc_id, op.terms);
        } else {
            ok = (index.search_and(op.terms) == model.search(op.terms)) && ok;
        }
    }
    report(label, ok);
}

// Documents are added first (not timed), then the queries are answered and timed.
void test_big(const std::string& label, const std::vector<Op>& adds, const std::vector<Op>& queries) {
    InvertedIndex index;
    ListModel model;
    const auto build_start = std::chrono::steady_clock::now();
    for (const Op& op : adds) index.add_document(op.doc_id, op.terms);
    const auto build_time = std::chrono::steady_clock::now() - build_start;
    for (const Op& op : adds) model.add(op.doc_id, op.terms);

    std::vector<std::vector<int>> answers(queries.size());
    const auto query_start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < queries.size(); ++i) answers[i] = index.search_and(queries[i].terms);
    const auto query_time = std::chrono::steady_clock::now() - query_start;

    bool correct = true;
    for (std::size_t i = 0; i < queries.size() && correct; ++i) correct = answers[i] == model.search(queries[i].terms);
    const bool fast = build_time < std::chrono::seconds(1) && query_time < std::chrono::seconds(1);
    report(label + (fast ? "" : " (too slow)"), correct && fast);
}

std::string word(int k) { return "w" + std::to_string(k); }

void test_all() {
	test_script("description example",
		{D(1, {"red", "apple"}), D(2, {"green", "apple"}), D(3, {"red", "pear"}), Q({"red"}), Q({"apple"}), Q({"red", "apple"}), Q({"green", "pear"}), Q({"blue"})});
	test_script("an empty index",
		{Q({"a"}), Q({"a", "b"})});
	test_script("an unknown term makes the result empty",
		{D(1, {"a", "b"}), D(2, {"a"}), Q({"a", "zzz"}), Q({"zzz", "a"}), Q({"zzz"})});
	test_script("a single term returns all its documents in order",
		{D(3, {"x"}), D(5, {"y"}), D(8, {"x", "y"}), D(9, {"x"}), Q({"x"}), Q({"y"})});
	test_script("a term repeated inside one document counts once",
		{D(1, {"a", "a", "a"}), D(2, {"a", "b", "a", "b"}), Q({"a"}), Q({"a", "b"}), Q({"b"})});
	test_script("a term repeated in the query",
		{D(1, {"a", "b"}), D(2, {"a"}), Q({"a", "a"}), Q({"a", "b", "a", "b"}), Q({"b", "b", "b"})});
	test_script("a document without terms",
		{D(1, {}), D(2, {"a"}), Q({"a"}), Q({"b"})});
	test_script("an empty query returns nothing",
		{D(1, {"a"}), D(2, {"b"}), Q({})});
	test_script("all terms in the same documents",
		{D(1, {"a", "b", "c"}), D(2, {"a", "b", "c"}), D(3, {"a", "b", "c"}), Q({"a", "b", "c"}), Q({"c", "b", "a"})});
	test_script("terms that never meet",
		{D(1, {"a"}), D(2, {"b"}), D(3, {"c"}), Q({"a", "b"}), Q({"a", "c"}), Q({"b", "c"}), Q({"a", "b", "c"})});
	test_script("the order of the query terms does not matter",
		{D(1, {"a", "b", "c"}), D(2, {"a", "c"}), D(3, {"b", "c"}), D(4, {"a", "b", "c"}), Q({"a", "b", "c"}), Q({"c", "a", "b"}), Q({"b", "c", "a"})});
	test_script("a rare term and a common term",
		{D(1, {"common"}), D(2, {"common"}), D(3, {"common"}), D(4, {"common"}), D(5, {"common"}), D(6, {"common"}), D(7, {"common"}), D(8, {"common"}), D(9, {"common"}), D(10, {"common"}), D(11, {"common"}), D(12, {"common"}), D(13, {"common"}), D(14, {"common"}), D(15, {"common"}), D(16, {"common"}), D(17, {"common"}), D(18, {"common"}), D(19, {"common"}), D(20, {"common"}), D(21, {"common", "rare"}), D(22, {"common"}), D(23, {"common", "rare"}), Q({"common", "rare"}), Q({"rare", "common"}), Q({"common"})});
	test_script("document ids are not consecutive",
		{D(10, {"a"}), D(20, {"a", "b"}), D(1000, {"b"}), D(2000000000, {"a", "b"}), Q({"a"}), Q({"b"}), Q({"a", "b"})});
	test_script("a document id equal to zero and large ids",
		{D(0, {"a"}), D(1, {"a"}), D(2147483647, {"a"}), Q({"a"})});
	test_script("case and punctuation are part of the term",
		{D(1, {"Apple", "apple", "apple!"}), D(2, {"apple"}), Q({"apple"}), Q({"Apple"}), Q({"apple!"}), Q({"Apple", "apple"})});
	test_script("queries are interleaved with insertions",
		{D(1, {"a"}), Q({"a"}), D(2, {"a", "b"}), Q({"a"}), Q({"a", "b"}), D(3, {"a", "b"}), Q({"a", "b"}), Q({"b"})});
	test_script("long and empty strings as terms",
		{D(1, {"", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}), D(2, {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}), Q({""}), Q({"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}), Q({"", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"})});
	test_script("three terms with different list sizes",
		{D(1, {"t1"}), D(2, {"t1", "t2"}), D(3, {"t1"}), D(4, {"t1", "t2"}), D(5, {"t1"}), D(6, {"t1", "t2"}), D(7, {"t1", "t3"}), D(8, {"t1", "t2"}), D(9, {"t1"}), D(10, {"t1", "t2"}), D(11, {"t1"}), D(12, {"t1", "t2"}), D(13, {"t1"}), D(14, {"t1", "t2", "t3"}), D(15, {"t1"}), D(16, {"t1", "t2"}), D(17, {"t1"}), D(18, {"t1", "t2"}), D(19, {"t1"}), D(20, {"t1", "t2"}), D(21, {"t1", "t3"}), D(22, {"t1", "t2"}), D(23, {"t1"}), D(24, {"t1", "t2"}), D(25, {"t1"}), D(26, {"t1", "t2"}), D(27, {"t1"}), D(28, {"t1", "t2", "t3"}), D(29, {"t1"}), D(30, {"t1", "t2"}), D(31, {"t1"}), D(32, {"t1", "t2"}), D(33, {"t1"}), D(34, {"t1", "t2"}), D(35, {"t1", "t3"}), D(36, {"t1", "t2"}), D(37, {"t1"}), D(38, {"t1", "t2"}), D(39, {"t1"}), D(40, {"t1", "t2"}), D(41, {"t1"}), D(42, {"t1", "t2", "t3"}), D(43, {"t1"}), D(44, {"t1", "t2"}), D(45, {"t1"}), D(46, {"t1", "t2"}), D(47, {"t1"}), D(48, {"t1", "t2"}), D(49, {"t1", "t3"}), Q({"t1", "t2", "t3"}), Q({"t3", "t2", "t1"}), Q({"t2", "t3"}), Q({"t1", "t3"})});
	test_script("random 1: 20 operations over 6 terms",
		{D(1, {"a", "d"}), Q({"f", "d", "zz"}), D(3, {}), Q({"d", "e", "zz"}), Q({"c", "f", "zz"}), D(4, {"a", "a"}), D(7, {}), Q({"f", "b", "d"}), Q({"b", "zz", "d", "d"}), Q({"b", "f"}), D(9, {"a", "d"}), Q({"f", "a", "b", "f"}), Q({"a", "f"}), D(12, {"e", "f", "b"}), D(15, {"e", "d", "e"}), Q({"b", "f", "zz"}), D(18, {"c"}), D(21, {"a", "d"}), Q({}), Q({"zz", "d", "c", "d"})});
	test_script("random 2: 50 operations over 6 terms",
		{Q({"a", "c", "f"}), Q({"e", "e", "d", "f"}), D(3, {"a"}), Q({"zz", "e", "b", "d"}), D(6, {"d", "c"}), Q({"f", "a", "d", "zz"}), Q({"zz", "b", "e", "zz"}), Q({"a", "d", "zz"}), D(9, {"e"}), D(11, {"c", "a", "e"}), D(14, {"d", "e"}), D(15, {"e"}), Q({}), Q({"a", "zz"}), Q({}), D(16, {"a", "c", "b"}), D(19, {"c"}), D(20, {"c"}), D(21, {"f", "f"}), D(24, {"d", "d"}), D(26, {"c", "d", "b"}), D(28, {"b", "e", "d", "a"}), D(30, {"a"}), Q({"d"}), Q({"e", "zz", "b"}), Q({"d", "b", "e", "f"}), D(33, {"c", "f", "f", "d"}), D(35, {"b"}), Q({"a", "zz"}), D(37, {"d"}), Q({"a"}), Q({}), Q({"e"}), D(40, {"e", "a", "d", "b"}), D(41, {"f", "d", "e", "b"}), D(44, {"c", "e", "d"}), D(47, {"c", "a", "b"}), D(49, {"b", "c", "d", "b"}), D(50, {"e", "c", "f"}), D(53, {"a"}), Q({}), D(54, {"b", "c", "c", "e"}), D(56, {"c", "c"}), D(57, {"f", "d", "b", "e"}), Q({}), D(59, {}), D(60, {"c"}), D(63, {"a", "e", "e"}), D(64, {"c", "c"}), Q({})});
	test_script("random 3: 120 operations over 6 terms",
		{D(2, {}), Q({"a", "e"}), Q({}), D(3, {"b"}), Q({"d", "b", "a", "d"}), D(4, {"f"}), Q({"d", "zz", "e"}), Q({"e", "c"}), Q({"a", "b"}), Q({}), D(6, {"c", "d", "d", "c"}), D(7, {"e", "d"}), D(8, {"e", "f", "d", "f"}), D(9, {"b", "c", "b", "b"}), D(11, {}), Q({"a", "f", "e"}), Q({"d"}), Q({}), D(13, {"c", "b", "c", "a"}), D(16, {"a", "b", "b", "a"}), Q({"a", "c", "e"}), Q({}), D(17, {"c", "d"}), D(18, {}), D(20, {}), D(23, {"b"}), Q({"zz"}), Q({"a", "f"}), D(26, {"b", "b"}), D(29, {}), Q({"zz", "f", "e", "zz"}), Q({"b"}), D(32, {"a"}), Q({"c"}), Q({"zz", "d", "e"}), D(34, {"d", "a", "d", "c"}), D(36, {}), Q({"e", "a", "a"}), Q({"b", "e", "b", "b"}), D(38, {"e", "d", "b"}), Q({"d"}), D(41, {"e", "f"}), Q({"b"}), D(44, {"b", "f", "d"}), D(47, {"f", "b"}), D(48, {"f", "c", "b", "e"}), Q({"c"}), D(50, {"c", "b", "f", "f"}), Q({"a", "zz", "a", "e"}), Q({"d", "b", "b", "c"}), D(53, {}), D(55, {"d", "e"}), Q({"f", "a", "e", "a"}), Q({}), D(56, {"e"}), Q({}), D(57, {"d", "d", "b"}), Q({"b", "e", "d"}), Q({}), D(60, {"a", "f", "c"}), D(62, {"a", "b", "e", "d"}), Q({}), Q({"b", "zz", "c", "b"}), D(63, {"b", "c", "c", "e"}), Q({"zz", "zz", "zz"}), Q({"e"}), D(65, {}), Q({"d", "b", "c", "zz"}), D(66, {}), Q({}), D(69, {"a"}), D(72, {"d", "e"}), Q({"c", "a", "a", "d"}), Q({"c", "e"}), D(75, {"d", "a", "f", "d"}), D(78, {}), Q({"f", "f", "zz", "f"}), D(80, {"e", "d", "f", "f"}), Q({"f", "b"}), D(83, {"b", "c", "e", "a"}), Q({"d", "d", "c", "zz"}), Q({}), D(86, {"f"}), Q({"f", "a"}), D(89, {"f"}), Q({"zz", "c", "zz"}), D(90, {"a", "c", "c", "f"}), D(93, {"c", "b", "d", "c"}), D(95, {"a", "c", "e", "a"}), Q({"a", "c", "a"}), Q({}), D(98, {"f"}), D(101, {"e", "c"}), D(102, {"c"}), D(103, {"f", "c", "d", "e"}), Q({}), D(106, {"c", "c", "e", "f"}), D(109, {"b", "d", "c"}), Q({"f", "b"}), D(112, {"f"}), D(115, {"c", "d", "b"}), Q({"a"}), Q({"zz"}), Q({"e", "f", "b"}), Q({"d", "e"}), D(116, {"c", "c", "d"}), D(119, {"f"}), Q({}), D(121, {"d", "a"}), Q({}), D(124, {}), Q({"f"}), D(127, {"f", "e", "d", "c"}), Q({"a", "e"}), Q({}), D(128, {"d", "d", "b"}), Q({"zz"}), D(131, {"d", "b", "d", "f"}), D(133, {"a", "b", "a", "a"}), D(134, {"c", "d", "e"})});
	test_script("random 4: 300 operations over 6 terms",
		{D(1, {"b", "f", "b"}), Q({}), D(2, {"e", "a", "e", "d", "c"}), D(4, {"c", "a", "a", "e", "a"}), D(5, {}), Q({}), D(6, {}), D(7, {"c", "f", "b", "f", "d"}), D(10, {"c", "f"}), Q({"a"}), Q({"b", "c", "d", "e"}), Q({"a", "c", "e", "d"}), D(13, {"d", "f", "a", "f"}), D(16, {"a", "c", "b", "a", "b"}), D(17, {"a", "a", "f"}), D(20, {"e", "c", "a"}), Q({}), D(21, {"f", "b", "d"}), Q({"a", "f", "e"}), D(23, {"a", "c"}), D(25, {}), Q({"f", "b"}), D(27, {}), Q({"a", "d"}), Q({"e", "b", "c", "c"}), D(29, {"d", "a", "b", "f"}), Q({"e", "f", "zz", "zz"}), Q({"e", "a", "zz", "c"}), Q({"c"}), D(31, {}), D(32, {"a", "a", "c", "f"}), D(34, {"c", "c"}), D(37, {"e", "e", "a", "e", "a"}), D(40, {"c", "e"}), D(42, {"d", "c", "f"}), Q({}), Q({}), D(45, {"e", "c", "b"}), Q({"c", "zz"}), Q({"c", "d", "e"}), Q({"e", "b", "a", "b"}), D(46, {"b", "a", "b", "d"}), Q({"a", "zz", "a", "e"}), Q({}), D(47, {"e", "e", "f", "a", "a"}), Q({"f"}), Q({"zz", "d", "a", "e"}), D(49, {"c", "b", "b", "e", "d"}), Q({"d"}), D(51, {"b", "d", "f", "a"}), Q({"d", "b"}), D(54, {"e", "d", "a"}), D(57, {"e", "d", "a", "c"}), Q({"a", "b", "c"}), Q({}), D(59, {"f", "c", "e", "f"}), Q({"c", "e", "d", "e"}), Q({"d", "e", "f", "e"}), D(61, {"e"}), D(62, {"b", "c", "f", "a"}), Q({"a", "c", "d", "d"}), D(65, {"a", "a", "a", "a", "d"}), D(67, {"f", "f"}), Q({"d", "d"}), Q({"c", "b", "d"}), D(68, {"c", "b"}), Q({"d", "c"}), Q({"f", "d"}), Q({"c", "zz", "d"}), D(70, {"f", "d", "a"}), D(71, {"b"}), Q({}), D(73, {}), D(76, {"a"}), D(79, {}), D(82, {"c", "a", "f"}), Q({}), Q({"zz", "f", "f"}), D(84, {"c", "b", "d", "f", "a"}), Q({}), Q({}), Q({"f", "e"}), D(86, {}), Q({"a", "b", "d"}), Q({"b"}), D(88, {"e", "c", "d", "f", "e"}), Q({"c", "zz", "d", "a"}), D(91, {"c", "f", "c", "a", "e"}), Q({"zz", "zz"}), D(94, {"c", "f"}), D(95, {"c"}), D(98, {"e", "a", "e", "e", "e"}), D(100, {"c", "d"}), Q({"d", "b"}), D(103, {"b", "c", "b"}), Q({"f"}), Q({"e", "b", "a"}), D(106, {"b", "f", "b", "c", "e"}), D(108, {}), D(109, {"c", "b", "a", "f", "e"}), Q({}), Q({}), D(110, {"e", "f", "c", "d"}), D(111, {"e", "b"}), D(112, {"f", "f"}), Q({"c", "e"}), Q({"a", "a", "c"}), D(113, {"b", "f"}), Q({"a", "a"}), Q({}), D(114, {"e", "a"}), D(115, {"d"}), D(118, {"f", "b", "a", "f", "c"}), D(121, {"a", "c"}), Q({"e"}), Q({"d", "a"}), Q({"f", "e", "d", "e"}), D(123, {"b", "f"}), D(124, {}), Q({}), D(125, {"c", "e", "a"}), D(127, {"c", "d", "b", "d"}), Q({"a", "f"}), Q({"e", "e"}), Q({"f", "e", "a", "e"}), D(130, {"b"}), D(133, {"f"}), Q({"d"}), Q({"c", "c"}), D(135, {"d", "a", "e"}), D(137, {"f", "f", "e", "a", "e"}), Q({"d"}), Q({}), D(139, {"f", "d", "c", "c"}), D(142, {"a", "a", "d"}), Q({}), Q({}), Q({"e", "zz", "c", "e"}), D(145, {"c", "d", "f", "b", "e"}), D(148, {"b", "a"}), Q({"d", "f"}), D(151, {"a", "e", "d", "d", "d"}), D(153, {"f", "b", "f"}), Q({"a"}), D(154, {"b", "e", "f", "f"}), D(156, {"a", "e", "a", "d", "b"}), D(157, {"f", "b", "a"}), D(159, {"b", "f", "d", "f", "d"}), D(162, {"f", "b", "d", "d", "d"}), D(165, {"c", "b", "b"}), D(167, {}), Q({}), Q({"zz", "d", "f"}), D(169, {"b"}), Q({}), Q({}), D(172, {"f", "b", "e", "f", "d"}), Q({"zz", "b", "d", "b"}), Q({"f", "b"}), D(175, {"b", "b", "d", "e"}), D(176, {"b", "a", "e"}), Q({"f", "e", "f", "e"}), D(178, {"a", "e", "f"}), D(179, {"a", "f", "d", "a"}), Q({"zz"}), D(181, {"c", "f", "d"}), D(182, {"f", "c", "e", "f"}), D(185, {"e", "b", "f"}), D(187, {"d"}), Q({"b", "c"}), Q({"zz"}), Q({}), Q({"b", "c"}), D(189, {"c", "e", "d"}), D(190, {"b", "b"}), D(193, {"e", "b", "e", "d"}), Q({"e"}), D(195, {"f", "b", "a"}), Q({}), D(198, {}), D(200, {"d", "f", "b"}), Q({"f"}), D(201, {"d"}), D(202, {"f", "d", "c", "f", "b"}), D(205, {"c"}), D(207, {}), D(209, {"f"}), D(211, {"e", "a", "e", "c"}), Q({"e", "a"}), D(213, {"b"}), Q({}), D(215, {"e", "b", "f", "b", "e"}), D(216, {"b", "d", "f", "e", "b"}), Q({"c", "f", "a", "f"}), D(217, {"d", "f", "d", "e"}), Q({"a", "f"}), D(220, {"e", "b", "d", "c"}), Q({"d", "c", "d"}), D(221, {"e"}), D(222, {"b", "d", "f"}), D(224, {"a"}), D(227, {}), Q({"b", "b", "e"}), D(230, {"e", "c", "b"}), D(233, {"a", "c", "a", "d"}), D(236, {"b"}), Q({"d", "a"}), Q({}), Q({"d", "a", "d", "zz"}), D(239, {"c", "d", "c", "c", "d"}), Q({}), Q({"a", "d", "c"}), Q({"zz", "b"}), Q({"zz", "c", "a", "zz"}), Q({"d", "d"}), Q({}), Q({}), D(242, {}), D(244, {"c", "e"}), D(246, {"a", "d", "f", "a"}), Q({"c", "e", "zz"}), D(247, {"c", "b"}), D(250, {"e"}), D(252, {"d", "c", "c", "c"}), Q({}), Q({"zz"}), Q({"e", "c", "e"}), Q({}), D(253, {"d", "a"}), D(256, {"f", "c", "b", "b", "a"}), D(258, {}), Q({"zz", "zz"}), Q({"zz"}), D(261, {"c", "c"}), Q({"a"}), D(263, {"a", "a", "c", "d", "f"}), D(266, {}), Q({"d", "b", "b", "b"}), D(267, {"e", "a", "f", "c"}), D(268, {"d", "c"}), Q({"f", "c", "b", "f"}), D(270, {}), D(273, {}), D(274, {"b", "a", "e", "b"}), D(276, {"d"}), D(278, {"f", "a", "b"}), Q({"f"}), Q({"d", "e", "d", "c"}), D(279, {}), Q({"f", "e", "zz", "d"}), Q({"c", "c", "a", "f"}), D(282, {"d", "e", "e", "f"}), Q({"zz", "d", "e", "f"}), Q({"e", "d", "b"}), Q({"c", "e", "zz", "zz"}), D(285, {"c", "c"}), D(286, {"d", "c", "b"}), D(287, {"d", "c", "f", "f", "b"}), D(289, {}), Q({"zz", "zz"}), Q({"zz", "e"}), Q({"d", "a"}), D(291, {"a", "b", "f", "a"}), D(294, {"a", "b", "f", "c", "d"}), Q({"a"}), Q({"zz"}), Q({"f", "e", "f", "c"}), D(296, {"e", "d", "a"}), Q({"b", "e", "a"}), Q({"b"}), Q({"b", "e", "b"}), Q({"f", "f"}), Q({}), Q({"d", "d"}), D(297, {}), Q({"e", "c", "zz"}), Q({"c", "e", "a"}), Q({"zz", "e"}), Q({"a", "zz", "c"}), Q({"a", "e", "b"}), Q({"d", "c"}), Q({}), D(300, {"e", "b", "b", "c"}), Q({}), D(303, {}), D(306, {}), D(307, {}), D(309, {}), Q({"b", "d", "b", "c"}), D(312, {"c", "b", "b", "b"}), D(313, {"e"}), Q({}), D(315, {}), D(316, {"f", "a", "b", "b"}), D(318, {}), D(320, {"b", "a", "d", "b", "b"}), D(322, {"a", "e"}), D(323, {"f"}), D(324, {"a"}), Q({})});
	test_script("random 5: 300 operations over 6 terms",
		{D(3, {}), Q({"e", "d"}), Q({}), Q({"c"}), D(5, {"e", "d"}), Q({"a", "d", "b"}), Q({"c", "b", "e"}), Q({"a"}), Q({"c", "e", "c"}), Q({"zz", "zz"}), D(7, {"c"}), D(9, {"a", "c"}), D(10, {"e"}), D(13, {"f", "b"}), D(16, {"b", "b"}), D(19, {"b"}), D(21, {}), D(23, {"a"}), D(26, {"d"}), Q({"f"}), D(29, {}), D(30, {}), Q({"f"}), D(32, {"c", "e"}), Q({"c", "f"}), D(34, {}), Q({"f", "zz", "b", "a"}), D(36, {"e"}), D(38, {}), D(40, {"e", "a"}), D(41, {}), D(43, {}), D(44, {"e"}), D(47, {"e", "d"}), Q({"e", "e", "d"}), D(49, {}), D(50, {"b"}), D(51, {}), Q({}), D(53, {}), D(55, {}), Q({"a"}), D(58, {}), Q({"e"}), Q({}), D(60, {"a", "a"}), Q({"e", "c", "b"}), D(63, {"f"}), Q({"f", "c", "e", "c"}), Q({"a", "d", "c", "d"}), Q({"a", "f"}), Q({"a", "b"}), Q({"c"}), D(65, {"d"}), D(66, {"b"}), Q({}), D(68, {"b", "c"}), D(71, {"e", "d"}), Q({"c", "f", "b", "c"}), D(72, {"c"}), D(75, {"b", "d"}), Q({"zz"}), D(78, {}), D(81, {}), D(82, {}), D(84, {"d", "f"}), D(87, {}), Q({"d", "a", "d"}), Q({"zz", "f", "d", "b"}), Q({"a", "zz"}), Q({"d", "b"}), Q({"c", "zz", "a", "d"}), Q({"b", "b"}), D(88, {"e"}), D(89, {"d", "d"}), D(90, {"a", "d"}), Q({"b", "d", "d", "a"}), D(91, {}), D(92, {"e", "e"}), D(95, {"d", "d"}), D(98, {"b"}), D(101, {"e"}), Q({"c"}), Q({}), Q({"a", "d"}), D(104, {"f", "b"}), Q({"d", "zz", "e"}), D(105, {}), D(106, {}), D(107, {"d"}), D(109, {"e", "a"}), Q({"d", "d", "d", "c"}), Q({"zz"}), Q({"zz"}), D(110, {"a", "e"}), D(112, {"c"}), Q({}), D(115, {"c"}), Q({"c", "e", "a"}), D(118, {"e"}), Q({"e", "b", "c"}), D(119, {"b", "b"}), Q({"d"}), Q({"a"}), Q({"a", "d", "b"}), Q({"c", "c", "d", "a"}), D(122, {"c"}), Q({"f", "e"}), D(124, {}), D(125, {"b"}), D(128, {}), D(130, {"f"}), Q({"f", "e"}), D(133, {"c"}), D(135, {"b", "b"}), D(136, {"b", "a"}), Q({"zz", "a", "c", "d"}), Q({"c", "c"}), Q({"b", "zz", "zz"}), Q({"c"}), D(139, {"f"}), Q({}), D(141, {"e", "b"}), D(144, {"d"}), D(146, {}), D(148, {}), Q({"a"}), D(151, {}), Q({"d"}), D(154, {"f", "e"}), Q({"b"}), D(157, {}), Q({"a"}), D(160, {}), D(162, {"f", "f"}), Q({"b", "a", "c", "zz"}), D(163, {"d", "e"}), D(166, {}), D(168, {"b", "b"}), D(169, {"f"}), D(170, {"d"}), D(172, {}), Q({"f"}), Q({}), Q({"a", "c"}), Q({"a"}), Q({}), Q({"a", "d"}), Q({"e", "f", "f", "d"}), Q({"d"}), D(174, {"d"}), D(177, {}), D(178, {}), D(179, {"d", "d"}), Q({"e", "a", "a"}), D(182, {}), D(184, {"b"}), Q({"d", "zz"}), D(187, {}), D(188, {"e", "c"}), Q({"a", "zz"}), Q({}), D(189, {}), D(190, {"a"}), D(191, {"a"}), Q({"d", "a", "d", "e"}), Q({}), D(194, {"d", "f"}), Q({"b", "e", "d"}), D(197, {"d"}), D(198, {"e", "f"}), D(199, {"a"}), Q({"a"}), Q({"a", "a", "e", "d"}), Q({"a"}), D(202, {}), Q({"c", "d", "f", "a"}), Q({}), D(205, {"d"}), D(207, {"d"}), D(209, {"b", "f"}), Q({"c", "e", "e"}), D(212, {"d", "f"}), Q({"e", "d", "d", "zz"}), Q({"a", "a"}), Q({"zz", "a", "zz"}), D(214, {"c", "b"}), D(215, {"a"}), D(218, {"c", "e"}), D(221, {}), D(223, {"a", "d"}), D(226, {"f"}), Q({}), Q({}), D(228, {"e", "f"}), Q({"d", "a", "e", "d"}), D(231, {"a", "b"}), D(234, {}), Q({}), Q({"zz", "zz"}), D(237, {}), D(238, {}), D(241, {"d", "c"}), Q({"e", "zz"}), D(244, {"a"}), Q({}), D(246, {"d"}), D(247, {}), D(250, {}), Q({"zz", "d", "c"}), Q({"b", "b", "zz", "b"}), D(251, {"a"}), Q({"zz", "b"}), D(252, {"f", "e"}), D(254, {"f", "e"}), D(256, {"f"}), D(257, {}), Q({"f", "a", "zz"}), Q({"e", "zz", "a"}), D(259, {"a", "c"}), Q({"b", "e", "c"}), D(260, {"e"}), D(262, {"f"}), Q({"f", "e", "d", "d"}), Q({"zz", "zz", "a"}), Q({"d"}), D(264, {"e"}), D(265, {}), Q({"e", "d", "c"}), D(266, {"b"}), Q({"zz", "c", "f", "f"}), D(269, {"a", "c"}), D(270, {"b"}), Q({"f", "d", "a"}), D(273, {}), Q({"d", "d", "zz", "zz"}), Q({"a", "d", "zz", "a"}), D(274, {}), Q({"a", "c", "e"}), D(275, {}), D(276, {"c", "c"}), D(279, {"e"}), D(281, {}), Q({"e", "b"}), D(282, {}), Q({"c"}), Q({"zz", "e"}), D(284, {"a", "f"}), D(287, {}), D(288, {}), D(290, {}), D(293, {"a", "c"}), D(296, {}), D(298, {}), D(299, {"c", "b"}), Q({"e", "zz"}), Q({"a", "a"}), D(300, {}), D(303, {"d", "a"}), Q({"d"}), D(304, {}), D(306, {}), D(308, {"c"}), Q({"c", "a"}), D(309, {"d", "a"}), D(311, {"f"}), D(312, {"a", "b"}), D(315, {}), Q({}), D(318, {}), Q({"b", "zz", "e"}), Q({}), D(319, {}), Q({"e", "a"}), D(321, {}), D(324, {"c"}), D(327, {"d"}), D(330, {"c"}), D(332, {"d", "b"}), Q({"e"}), D(333, {"b"}), D(335, {}), Q({"d", "zz", "e", "zz"}), D(338, {}), D(340, {"d", "e"}), D(341, {}), D(344, {"c"}), Q({}), Q({"b", "e"}), D(347, {}), D(348, {"b"}), D(350, {"a"}), D(351, {}), Q({"d", "b", "zz"}), D(353, {"a", "f"}), D(354, {}), D(357, {"d"}), D(359, {"e"}), D(361, {"d"}), D(364, {})});

	// ---- big tests ----
	{
		// w0 is in 80% of the documents, w1 in 40%, ..., rare words in a few documents
		std::mt19937 gen(1);
		std::vector<Op> adds, queries;
		for (int id = 1; id <= 200000; ++id) {
			std::vector<std::string> terms;
			if (gen() % 100 < 80) terms.push_back("common");
			if (gen() % 100 < 40) terms.push_back("often");
			if (gen() % 10000 < 4) terms.push_back("rare" + std::to_string(gen() % 50));
			terms.push_back(word(static_cast<int>(gen() % 100000)));
			adds.push_back(D(id, terms));
		}
		for (int i = 0; i < 20000; ++i) {
			std::vector<std::string> q{"common", "rare" + std::to_string(gen() % 50)};
			if (i % 3 == 0) q.push_back("often");
			queries.push_back(Q(q));
		}
		test_big("big: 200000 documents, a very common term AND a rare one", adds, queries);
	}
	{
		std::vector<Op> adds, queries;
		for (int id = 1; id <= 100000; ++id) {
			std::vector<std::string> terms{"everywhere"};
			if (id % 10000 == 0) terms.push_back("sparse");
			adds.push_back(D(id, terms));
		}
		for (int i = 0; i < 20000; ++i) queries.push_back(Q({"everywhere", "sparse"}));
		test_big("big: one term in every document, one in ten documents", adds, queries);
	}
	{
		std::mt19937 gen(2);
		std::vector<Op> adds, queries;
		for (int id = 1; id <= 100000; ++id) {
			std::vector<std::string> terms;
			for (int k = 0; k < 4; ++k) if (gen() % 2 == 0) terms.push_back(word(k));
			adds.push_back(D(id, terms));
		}
		for (int i = 0; i < 300; ++i) queries.push_back(Q({word(0), word(1), word(static_cast<int>(2 + i % 2))}));
		test_big("big: three terms, each in half of 100000 documents, large results", adds, queries);
	}
	{
		// each document repeats its terms many times
		std::vector<Op> adds, queries;
		for (int id = 1; id <= 20000; ++id) {
			std::vector<std::string> terms;
			for (int r = 0; r < 100; ++r) { terms.push_back("alpha"); terms.push_back("beta"); }
			adds.push_back(D(id, terms));
		}
		for (int i = 0; i < 100; ++i) queries.push_back(Q({"alpha", "beta"}));
		test_big("big: 20000 documents that repeat their terms 100 times", adds, queries);
	}
	{
		// a huge vocabulary, queries of unknown terms
		std::vector<Op> adds, queries;
		for (int id = 1; id <= 300000; ++id) adds.push_back(D(id, {word(id), word(id + 1)}));
		for (int i = 0; i < 100000; ++i) queries.push_back(Q({word(i * 3 + 1), word(i * 3 + 2)}));
		for (int i = 0; i < 100000; ++i) queries.push_back(Q({"missing" + std::to_string(i)}));
		test_big("big: 300000 documents with distinct terms, 200000 queries", adds, queries);
	}
}

}  // namespace

int main() {
	test_all();
	return 0;
}
