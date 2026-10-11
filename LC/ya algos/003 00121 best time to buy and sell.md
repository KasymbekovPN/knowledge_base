
# 121. Best Time to Buy and Sell Stock (Easy)

Дан массив `prices`, где `prices[i]` это цена акции в день `i`. Нужно выбрать **один день для покупки** и **более поздний день для продажи** так, чтобы **прибыль была максимальной**, и вернуть эту прибыль.

Если получить положительную прибыль невозможно, нужно вернуть `0` (то есть можно вообще ничего не покупать).

Продавать можно только **после** покупки: покупка и продажа в один день не приносят прибыли, а продажа раньше покупки недопустима.

**Ограничения:** `1 ≤ prices.length ≤ 10⁵`, `0 ≤ prices[i] ≤ 10⁴`.

**Примеры:**

```
Вход:  prices = [7, 1, 5, 3, 6, 4]
Выход: 5
Пояснение: купить в день 2 (цена 1), продать в день 5 (цена 6), прибыль 6 - 1 = 5.
           Покупка по 7 и продажа по 1 невозможна: продавать нужно позже покупки.
```

```
Вход:  prices = [7, 6, 4, 3, 1]
Выход: 0
Пояснение: цены только падают, прибыль невозможна, поэтому ответ 0.
```

Ориентир по времени: 15 минут. По протоколу: 2-3 примера с крайними случаями (один день, цены только растут, цены только падают, одинаковые цены), подход и сложность вслух, затем код в блокноте с тестами. Пришлите код, разберу.

---

Работа: 12:32 - 12:57
Доработка:

---

- min_price
- best_profit{0}

- [[7], 1, 5, 3, 6, 4] min_price : 7 best_profit: 0
- [7, [1], 5, 3, 6, 4] min_price : 1 best_profit: 0
- [7, 1, [5], 3, 6, 4] min_price : 1 best_profit: 4
- [7, 1, 5, [3], 6, 4] min_price : 1 best_profit: 4
- [7, 1, 5, 3, [6], 4] min_price : 1 best_profit: 5
- [7, 1, 5, 3, 6, [4]] min_price : 1 best_profit: 5

---

```cpp
#include <vector>
#include <string>
#include <iostream>
#include <format>
#include <algorithm>

namespace {
	int get_best_profit(const std::vector<int>& prices) {
		int best_profit{0};
		if (prices.empty()) return best_profit;
		
		auto min_price{prices[0]};
		for (int i{1}; i < static_cast<int>(prices.size()); ++i) {
			const auto current_price{prices[i]};
			best_profit = std::max(best_profit, current_price - min_price);
			min_price = std::min(min_price, current_price);
		}
	
		return best_profit;
	}
	
	void test_best_profit(const std::string& label,
						  const std::vector<int>& prices,
						  const int expected_profit) {
		const auto result{get_best_profit(prices)};
		std::cout << std::format("{}: {}\n",
			label,
			result == expected_profit ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	test_best_profit("has profit 1", {7, 1, 5, 3, 6, 4}, 5);
	test_best_profit("has no profit", {7, 6, 4, 3, 1}, 0);
	test_best_profit("no one day", {}, 0);
	test_best_profit("one day", {42}, 0);
	test_best_profit("has profit 2", {7, 2, 5, 3, 6, 4, 5, 4, 8, 1}, 6);
	test_best_profit("has profit 3", {7, 2, 5, 6, 9, 1, 5}, 7);

	return 0;
}

```