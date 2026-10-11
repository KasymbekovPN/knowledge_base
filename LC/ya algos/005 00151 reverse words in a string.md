
# 151. Reverse Words in a String (Medium)

Дана строка `s`. Нужно **развернуть порядок слов** в ней и вернуть получившуюся строку.

**Слово** это последовательность символов без пробелов. В `s` слова могут разделяться **одним или несколькими пробелами**, а также строка может начинаться и заканчиваться пробелами.

В результате слова должны идти в обратном порядке и разделяться **ровно одним пробелом**, **без** ведущих и замыкающих пробелов.

**Ограничения:** `1 ≤ s.length ≤ 10⁴`, строка содержит латинские буквы, цифры и пробелы, и в ней есть **хотя бы одно слово**.

**Примеры:**

```
Вход:  s = "the sky is blue"
Выход: "blue is sky the"
```

```
Вход:  s = "  hello world  "
Выход: "world hello"
Пояснение: ведущие и замыкающие пробелы в ответе убираются.
```

```
Вход:  s = "a good   example"
Выход: "example good a"
Пояснение: несколько пробелов между словами сводятся к одному.
```

**Дополнительный вопрос из условия:** если в вашем языке строка изменяема, можно ли решить задачу за **O(1) дополнительной памяти** (на месте)?

Ориентир по времени: 25 минут. Начните с примеров и крайних случаев: одно слово, только пробелы между словами, пробелы по краям, один символ. Назовите подход и сложность вслух: сначала простой вариант (разбить по пробелам и собрать в обратном порядке), затем in-place вариант. Затем код в блокноте с тестами. Пришлите код, разберу.

---

работа: 14:39 - 15:32

---

- [a]@good@@@example => a@good@@@example
- a[@]good@@@example => @agood@@@example
- @a[g]ood@@@example => g@aood@@@example
- g@a[o]od@@@example => go@aod@@@example
- go@a[o]d@@@example => goo@ad@@@example
- goo@a[d]@@@example => good@a@@@example
- good@a[@]@@example => @good@a@@example
- @good@a[@]example => @good@aexample
- @good@a[e]xample => e@good@axample
- e@good@a[x]ample => ex@good@aample
- ex@good@a[a]mple => exa@good@ample
- exa@good@a[m]ple => exam@good@aple
- exam@good@a[p]le => examp@good@ale
- examp@good@a[l]e => exampl@good@ae
- exampl@good@a[e] => example@good@a

---
- hello => hello

--- 

- "@@hello@@" => "hello"

---

- [@]@aa@@bb@@ => @aa@@bb@@
- [@]aa@@bb@@ => aa@@bb@@
- [a]a@@bb@@ => aa@@bb@@
- a[a]@@bb@@ => a[a]@@bb@@
- aa[@]@bb@@ => @aa@bb@@
- @aa[@]bb@@ => @aabb@@
- @aa[b]b@@ => b@aab@@
- b@aa[b]@@ => bb@aa@@
- bb@aa[@]@ => @bb@aa@
- @bb@aa[@] => @bb@aa
- !!! @bb@aa => bb@aa

---

- [@]@aa@@bb@@ => @aa@@bb@@
- [@]aa@@bb@@ => aa@@bb@@
- [a]a@@bb@@ => a@@bb@@a
- [a]@@bb@@a => @@bb@@aa
- [@]@bb@@aa => @bb@@aa@
- [@]bb@@aa@ => bb@@aa@
- [b]b@@aa@ => b@@aa@b
- [b]@@aa@b => @@aa@bb
- [@]@aa@bb => @aa@bb@
- [@]aa@bb@ => aa@bb@
- !!! aa@bb@ => aa@bb

---
- 13
- [@]@ab@@cd@ef@@ => @ab@@cd@ef // 12
- [@]ab@@cd@ef => ab@@cd@ef // 11
- [a]b@@cd@ef => b@@cd@ef@a // 10

---

```cpp
#include <cstddef>
#include <string>
#include <iostream>
#include <format>
#include <algorithm>

namespace {
	constexpr char DELIMITER{' '};

	void reverse_words(std::string& line) {
		
		std::size_t write_idx{0};
		for (std::size_t read_idx{0}; read_idx < line.size(); ++read_idx) {
			if (line[read_idx] == DELIMITER) continue;
			if (write_idx != 0) line[write_idx++] = DELIMITER;
			while (read < line.size() && line[read_idx] != DELIMITER) {
				line[write_idx++] = line[read_idx++];
			}
		}
		line.resize(write_idx);
		
		sdt::reverse(line.begin(), line.end());
		
		std::size_t start{0};
		while (start < line.size()) {
			std::size_t end{line.find(DELIMITER, start)};
			if (end == std::string::npos) end = line.size;
			std::reverse(line.begin() + start, line.begin() + end);
			start = end + 1;
		}
	}
	
	void test_reverse_wordrs(const std::string& label,
							 std::string& line,
							 const std::string& expected) {
		reverse_words(line);
		std::cout << std::format("{}: {}\n",
			label,
			line == expected ? "OK" : "FAIL") << std::flush;
	}
}

int main() {
	std::string input0{"the sky is blue"};
	test_reverse_words("test 0", input0, "blue is sky the");
	
	std::string input1{"  hello world  "};
	test_reverse_words("test 1", input1, "world hello");

	std::string input2{"a good   example"};
	test_reverse_words("test 2", input2, "example good a");

	return 0;
}

```
