**Задача 5 блока 4 (сортировки): Largest Number**

**Условие.** Дан массив неотрицательных целых чисел `nums`. Расположите числа в таком порядке, чтобы после склейки их десятичных записей получилось наибольшее возможное число. Верните его строкой, потому что результат может не поместиться ни в один целочисленный тип.

**Сигнатура:**

```cpp
std::string largest_number(std::vector<int>& nums);
```

**Ограничения:**

- 1 ≤ n ≤ 200 000
- 0 ≤ nums[i] ≤ 10^9

**Примеры:**

- `[10, 2]` → `"210"`
- `[3, 30, 34, 5, 9]` → `"9534330"`

**Вопросы интервьюера (ответьте после решения):**

1. Почему нельзя просто отсортировать числа по убыванию или по убыванию строк?
2. Какой компаратор вы выбрали и почему он корректен? Нужна ли ему транзитивность, и что будет с `std::sort`, если её нет?
3. Что вернуть для `[0, 0]`?
4. Какая сложность по времени и по памяти? Зависит ли она от длины записи чисел?

**Цель:** O(n log n) сравнений. Одно сравнение стоит O(длина записи числа), то есть до 10 символов.

**Шаблон:**

```cpp
namespace {
	std::string largest_number([[maybe_unused]] std::vector<int>& nums) {
		return {};
	}

	std::string expected_largest(const std::vector<int>& nums) {
		// key: the number repeated up to 20 characters; a larger key goes first
		std::vector<std::pair<std::string, std::string>> items;
		items.reserve(nums.size());
		for (const int value : nums) {
			const std::string text{std::to_string(value)};
			std::string key;
			while (key.size() < 20) key += text;
			key.resize(20);
			items.emplace_back(key, text);
		}
		std::ranges::sort(items);
		std::ranges::reverse(items);

		std::string result;
		for (const auto& item : items) result += item.second;
		return result[0] == '0' ? "0" : result;
	}

	void test_largest(const std::string& label,
					  std::vector<int> nums) {
		const auto expected{expected_largest(nums)};

		const auto result{largest_number(nums)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_largest("description example 1",
		{10, 2});
	test_largest("description example 2",
		{3, 30, 34, 5, 9});
	test_largest("single element 1",
		{1});
	test_largest("single element 0",
		{0});
	test_largest("single element 1000000000",
		{1000000000});
	test_largest("two zeros",
		{0, 0});
	test_largest("three zeros",
		{0, 0, 0});
	test_largest("zero and nine",
		{0, 9});
	test_largest("zeros and a nine",
		{9, 0, 0});
	test_largest("prefix pair: 10 and 100",
		{10, 100});
	test_largest("prefix pair: 100 and 10",
		{100, 10});
	test_largest("prefix pair: 12 and 121",
		{12, 121});
	test_largest("prefix pair: 121 and 12",
		{121, 12});
	test_largest("prefix pair: 830 and 8308",
		{830, 8308});
	test_largest("prefix pair: 8308 and 830",
		{8308, 830});
	test_largest("prefix pair: 824 and 8247",
		{824, 8247});
	test_largest("prefix pair: 432 and 43243",
		{432, 43243});
	test_largest("prefix pair: 34323 and 3432",
		{34323, 3432});
	test_largest("same digits, different lengths: 1 10 100 1000",
		{1, 10, 100, 1000});
	test_largest("same digits, reversed input",
		{1000, 100, 10, 1});
	test_largest("9 99 999",
		{999, 99, 9});
	test_largest("5 50 56",
		{5, 50, 56});
	test_largest("2 20 200",
		{2, 20, 200});
	test_largest("1 and 10",
		{10, 1});
	test_largest("2 and 20",
		{20, 2});
	test_largest("11 111 1",
		{11, 111, 1});
	test_largest("121 12 1212",
		{121, 12, 1212});
	test_largest("111311 and 1113",
		{111311, 1113});
	test_largest("equal numbers",
		{7, 7, 7});
	test_largest("equal big numbers",
		{1000000000, 1000000000});
	test_largest("extreme values",
		{1000000000, 999999999, 999999998});
	test_largest("numeric order is not the answer",
		{34, 3, 30});
	test_largest("lexicographic order is not the answer",
		{3, 30, 34, 5, 9, 31, 32});
	test_largest("single digits",
		{5, 3, 9, 1, 0, 7});
	test_largest("two-digit numbers",
		{10, 20, 30, 99, 98, 97});
	test_largest("ones and tens",
		{1, 10, 1, 10, 1, 10});
	test_largest("zeros with one other number",
		{0, 0, 0, 5, 0});
	test_largest("leading zero in the answer is impossible",
		{0, 0, 1});
	test_largest("a number and its repeated pattern",
		{12, 1212, 121212});
	test_largest("random 1, n=3",
		{0, 2, 211});
	test_largest("random 2, n=4",
		{312, 21, 10, 3});
	test_largest("random 3, n=2",
		{0, 0});
	test_largest("random 4, n=6",
		{121, 1, 3, 332, 1, 1});
	test_largest("random 5, n=3",
		{1, 2, 12});
	test_largest("random 6, n=7",
		{121, 13, 211, 11, 112, 2, 1});
	test_largest("random 7, n=7",
		{2, 3, 12, 1122, 20, 221, 3331});
	test_largest("random 8, n=5",
		{11, 2, 31, 2201, 10});

	{
		// n = 200000, pseudo-random values in [0, 1000000000]
		std::vector<int> nums(200000);
		unsigned state{179};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 2) % 1000000001u);
		}
		test_largest("n = 200000, random", nums);
	}
	{
		// n = 200000, pseudo-random values in [0, 99]
		std::vector<int> nums(200000);
		unsigned state{5};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 100u);
		}
		test_largest("n = 200000, random two-digit", nums);
	}
	{
		// n = 200000, all values equal 1000000000
		test_largest("n = 200000, all equal 1000000000", std::vector<int>(200000, 1000000000));
	}
	{
		// n = 200000, all zeros
		test_largest("n = 200000, all zeros", std::vector<int>(200000, 0));
	}
	{
		// n = 200000, powers of ten: 1, 10, 100, ..., 1000000000 repeated
		std::vector<int> nums;
		int power{1};
		for (int i{0}; i < 200000; ++i) {
			nums.push_back(power);
			power = (power == 1000000000) ? 1 : power * 10;
		}
		test_largest("n = 200000, powers of ten", nums);
	}
	{
		// n = 200000, numbers made of the digits 3 and 0 (many prefix relations)
		std::vector<int> nums;
		const int values[]{3, 30, 300, 33, 303, 330, 3030, 3003, 33003, 30303};
		for (int i{0}; i < 200000; ++i) nums.push_back(values[i % 10]);
		test_largest("n = 200000, digits 3 and 0", nums);
	}
	{
		// n = 200000, repeating patterns of 12
		std::vector<int> nums;
		const int values[]{1, 12, 121, 1212, 12121, 121212, 1212121, 12121212, 121212121};
		for (int i{0}; i < 200000; ++i) nums.push_back(values[i % 9]);
		test_largest("n = 200000, patterns of 12", nums);
	}
	{
		// n = 200000, 9 / 99 / 999 / ... mixed with 8 / 88 / 888 / ...
		std::vector<int> nums;
		int nines{9};
		int eights{8};
		for (int i{0}; i < 200000; ++i) {
			nums.push_back((i % 2 == 0) ? nines : eights);
			if (i % 2 == 1) {
				nines = (nines >= 99999999) ? 9 : nines * 10 + 9;
				eights = (eights >= 88888888) ? 8 : eights * 10 + 8;
			}
		}
		test_largest("n = 200000, nines and eights", nums);
	}
	{
		// n = 200000, sorted ascending 1..200000
		std::vector<int> nums(200000);
		for (int i{0}; i < 200000; ++i) nums[static_cast<std::size_t>(i)] = i + 1;
		test_largest("n = 200000, 1..200000", nums);
	}
	{
		// n = 200000, one nonzero value among zeros
		std::vector<int> nums(200000, 0);
		nums[123456] = 7;
		test_largest("n = 200000, one nonzero among zeros", nums);
	}

	return 0;
}
```

Тесты проверены запуском:

- Заглушка даёт FAIL на всех 57.
- Эталон проходит 57 из 57, в том числе под `-fsanitize=address,undefined`.
- Ожидаемые значения для малых тестов я сверил с полным перебором перестановок.
- Сортировка по убыванию чисел и сортировка по убыванию строк дают FAIL на 34 и 30 тестах соответственно.
- Вариант без обработки нулей проваливает 4 теста.
- Компаратор с `>=` на малых тестах проходит, а на тесте «все равны» падает с `bad_alloc`.
- Пузырьковая сортировка на больших тестах не укладывается во время.

Когда будет решение, пришлите его, и я разберу его с запуском. После него блок 4 закрыт, и мы переходим к воскресенью (две новые задачи и час повторения).

---

work : 17:10 - 17:38

---

- 3, 30, 34, 5, 9
- 9, 5, 3, 34, 30 => 9533430
- 9, 5, 30, 34, 3 => 9530343
- 9, 5, 34, 30, 3 => 9534303
- 9, 5, 34, 3, 30 => 9534330

- 34, 3 => 334 | 343

- 432xx, 43243
- 432, 43243 => 43243243
- 43243, 432 => 43243432

---

```cpp
#include <iostream>  
#include <format>  
#include <vector>  
#include <string>  
#include <algorithm>  
#include <queue>

namespace {
	using type = int;
	using huge_type = __int128_t;
	constexpr type BASE{10};

	huge_type get_dec_offset(type value) {
		type dim{1};
		while (value >= BASE) {
			++dim;
			value /= BASE;
		}

		huge_type result{1};
		while (dim != 0) {
			result *= BASE;
			--dim;
		}

		return result;
	}

	struct Comparator {
		bool operator()(const type lhs, const type rhs) const {
			const auto lhs_offset{get_dec_offset(lhs)};
			const auto rhs_offset{get_dec_offset(rhs)};

			const auto lhs_first{
				static_cast<huge_type>(lhs) * rhs_offset + static_cast<huge_type>(rhs)};
			const auto rhs_first{
				static_cast<huge_type>(rhs) * lhs_offset + static_cast<huge_type>(lhs)};

			return lhs_first < rhs_first;
		}
	};

	std::string largest_number(std::vector<int>& nums) {

		std::priority_queue<type, std::vector<type>, Comparator> pq;
		for (const auto num: nums) {
			pq.push(static_cast<type>(num));
		}

		std::string result;
		bool was_not_zero{false};
		while (!pq.empty()) {
			const auto hop{pq.top()};
			pq.pop();

			if (hop != 0) was_not_zero = true;
			if (!was_not_zero) continue;

			result += std::to_string(hop);
		}

		return was_not_zero ? result : "0";
	}

	std::string expected_largest(const std::vector<int>& nums) {
		// key: the number repeated up to 20 characters; a larger key goes first
		std::vector<std::pair<std::string, std::string>> items;
		items.reserve(nums.size());
		for (const int value : nums) {
			const std::string text{std::to_string(value)};
			std::string key;
			while (key.size() < 20) key += text;
			key.resize(20);
			items.emplace_back(key, text);
		}
		std::ranges::sort(items);
		std::ranges::reverse(items);

		std::string result;
		for (const auto& item : items) result += item.second;
		return result[0] == '0' ? "0" : result;
	}

	void test_largest(const std::string& label,
					  std::vector<int> nums) {
		const auto expected{expected_largest(nums)};

		const auto result{largest_number(nums)};

		std::cout << std::format("{}: {}\n",
			label,
			result == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_largest("description example 1",
		{10, 2});
	test_largest("description example 2",
		{3, 30, 34, 5, 9});
	test_largest("single element 1",
		{1});
	test_largest("single element 0",
		{0});
	test_largest("single element 1000000000",
		{1000000000});
	test_largest("two zeros",
		{0, 0});
	test_largest("three zeros",
		{0, 0, 0});
	test_largest("zero and nine",
		{0, 9});
	test_largest("zeros and a nine",
		{9, 0, 0});
	test_largest("prefix pair: 10 and 100",
		{10, 100});
	test_largest("prefix pair: 100 and 10",
		{100, 10});
	test_largest("prefix pair: 12 and 121",
		{12, 121});
	test_largest("prefix pair: 121 and 12",
		{121, 12});
	test_largest("prefix pair: 830 and 8308",
		{830, 8308});
	test_largest("prefix pair: 8308 and 830",
		{8308, 830});
	test_largest("prefix pair: 824 and 8247",
		{824, 8247});
	test_largest("prefix pair: 432 and 43243",
		{432, 43243});
	test_largest("prefix pair: 34323 and 3432",
		{34323, 3432});
	test_largest("same digits, different lengths: 1 10 100 1000",
		{1, 10, 100, 1000});
	test_largest("same digits, reversed input",
		{1000, 100, 10, 1});
	test_largest("9 99 999",
		{999, 99, 9});
	test_largest("5 50 56",
		{5, 50, 56});
	test_largest("2 20 200",
		{2, 20, 200});
	test_largest("1 and 10",
		{10, 1});
	test_largest("2 and 20",
		{20, 2});
	test_largest("11 111 1",
		{11, 111, 1});
	test_largest("121 12 1212",
		{121, 12, 1212});
	test_largest("111311 and 1113",
		{111311, 1113});
	test_largest("equal numbers",
		{7, 7, 7});
	test_largest("equal big numbers",
		{1000000000, 1000000000});
	test_largest("extreme values",
		{1000000000, 999999999, 999999998});
	test_largest("numeric order is not the answer",
		{34, 3, 30});
	test_largest("lexicographic order is not the answer",
		{3, 30, 34, 5, 9, 31, 32});
	test_largest("single digits",
		{5, 3, 9, 1, 0, 7});
	test_largest("two-digit numbers",
		{10, 20, 30, 99, 98, 97});
	test_largest("ones and tens",
		{1, 10, 1, 10, 1, 10});
	test_largest("zeros with one other number",
		{0, 0, 0, 5, 0});
	test_largest("leading zero in the answer is impossible",
		{0, 0, 1});
	test_largest("a number and its repeated pattern",
		{12, 1212, 121212});
	test_largest("random 1, n=3",
		{0, 2, 211});
	test_largest("random 2, n=4",
		{312, 21, 10, 3});
	test_largest("random 3, n=2",
		{0, 0});
	test_largest("random 4, n=6",
		{121, 1, 3, 332, 1, 1});
	test_largest("random 5, n=3",
		{1, 2, 12});
	test_largest("random 6, n=7",
		{121, 13, 211, 11, 112, 2, 1});
	test_largest("random 7, n=7",
		{2, 3, 12, 1122, 20, 221, 3331});
	test_largest("random 8, n=5",
		{11, 2, 31, 2201, 10});

	{
		// n = 200000, pseudo-random values in [0, 1000000000]
		std::vector<int> nums(200000);
		unsigned state{179};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 2) % 1000000001u);
		}
		test_largest("n = 200000, random", nums);
	}
	{
		// n = 200000, pseudo-random values in [0, 99]
		std::vector<int> nums(200000);
		unsigned state{5};
		for (auto& x : nums) {
			state = state * 1103515245u + 12345u;
			x = static_cast<int>((state >> 8) % 100u);
		}
		test_largest("n = 200000, random two-digit", nums);
	}
	{
		// n = 200000, all values equal 1000000000
		test_largest("n = 200000, all equal 1000000000", std::vector<int>(200000, 1000000000));
	}
	{
		// n = 200000, all zeros
		test_largest("n = 200000, all zeros", std::vector<int>(200000, 0));
	}
	{
		// n = 200000, powers of ten: 1, 10, 100, ..., 1000000000 repeated
		std::vector<int> nums;
		int power{1};
		for (int i{0}; i < 200000; ++i) {
			nums.push_back(power);
			power = (power == 1000000000) ? 1 : power * 10;
		}
		test_largest("n = 200000, powers of ten", nums);
	}
	{
		// n = 200000, numbers made of the digits 3 and 0 (many prefix relations)
		std::vector<int> nums;
		const int values[]{3, 30, 300, 33, 303, 330, 3030, 3003, 33003, 30303};
		for (int i{0}; i < 200000; ++i) nums.push_back(values[i % 10]);
		test_largest("n = 200000, digits 3 and 0", nums);
	}
	{
		// n = 200000, repeating patterns of 12
		std::vector<int> nums;
		const int values[]{1, 12, 121, 1212, 12121, 121212, 1212121, 12121212, 121212121};
		for (int i{0}; i < 200000; ++i) nums.push_back(values[i % 9]);
		test_largest("n = 200000, patterns of 12", nums);
	}
	{
		// n = 200000, 9 / 99 / 999 / ... mixed with 8 / 88 / 888 / ...
		std::vector<int> nums;
		int nines{9};
		int eights{8};
		for (int i{0}; i < 200000; ++i) {
			nums.push_back((i % 2 == 0) ? nines : eights);
			if (i % 2 == 1) {
				nines = (nines >= 99999999) ? 9 : nines * 10 + 9;
				eights = (eights >= 88888888) ? 8 : eights * 10 + 8;
			}
		}
		test_largest("n = 200000, nines and eights", nums);
	}
	{
		// n = 200000, sorted ascending 1..200000
		std::vector<int> nums(200000);
		for (int i{0}; i < 200000; ++i) nums[static_cast<std::size_t>(i)] = i + 1;
		test_largest("n = 200000, 1..200000", nums);
	}
	{
		// n = 200000, one nonzero value among zeros
		std::vector<int> nums(200000, 0);
		nums[123456] = 7;
		test_largest("n = 200000, one nonzero among zeros", nums);
	}

	return 0;
}
```


