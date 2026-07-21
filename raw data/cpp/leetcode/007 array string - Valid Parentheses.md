[[raw data/cpp/interview/_|<=]]

## Valid Parentheses

**Условие:** дана строка `s`, содержащая только символы `'(', ')', '{', '}', '[', ']'`. Определить, является ли строка "валидной" — каждая открывающая скобка должна быть закрыта скобкой того же типа, и скобки должны быть закрыты в правильном порядке (корректная вложенность).

### Идея

Классическая задача на **стек**. Идём по строке слева направо:

- если встретили открывающую скобку — кладём её в стек;
- если встретили закрывающую — она должна соответствовать скобке на вершине стека (последней открытой и ещё не закрытой). Если стек пуст или верхний элемент не пара — строка невалидна.

В конце стек должен быть пуст (все открытые скобки закрыты).

### Решение

```cpp
#include "valid_parentheses.h"  
  
#include <stack>  
#include <unordered_map>  
#include <iostream>  
#include <format>  
  
namespace valid_parentheses {  
  
bool is_valid_0(const std::string& line) {  
    static const std::unordered_map<char, char> BRACKET_PAIRS {  
        {')', '('},  
        {']', '['},  
        {'}', '{'}  
    };  
    std::stack<char> stk;  
  
    for (const auto& c : line) {  
        if (c == '{' || c == '[' || c == '(') {  
            stk.push(c);  
        } else {  
            if (stk.empty() || stk.top() != BRACKET_PAIRS.at(c)) {  
                return false;  
            }            stk.pop();  
        }    }  
    return stk.empty();  
}  
  
bool is_valid_1(const std::string& line) {  
    std::stack<char> stk;  
    for (const auto& c: line) {  
        switch (c) {  
            case '(': case '[': case '{':  
                stk.push(c);  
                break;  
            case ')':  
                if (stk.empty() || stk.top() != '(') return false;  
                stk.pop();  
                break;  
            case ']':  
                if (stk.empty() || stk.top() != '[') return false;  
                stk.pop();  
                break;  
            case '}':  
                if (stk.empty() || stk.top() != '{') return false;  
                stk.pop();  
                break;  
            default: break;  
        }    }  
    return stk.empty();  
}  
  
void demo() {  
    const std::string GOOD_LINE{"{[()]}"};  
    const std::string BAD_LINE{"([)]"};  
  
    std::cout << std::format("is_valid_0 of {} => {}\n", GOOD_LINE, is_valid_0(GOOD_LINE));  
    std::cout << std::format("is_valid_0 of {} => {}\n", BAD_LINE, is_valid_0(BAD_LINE));  
    std::cout << std::format("is_valid_1 of {} => {}\n", GOOD_LINE, is_valid_1(GOOD_LINE));  
    std::cout << std::format("is_valid_1 of {} => {}\n", BAD_LINE, is_valid_1(BAD_LINE));  
}  
  
}
```

### Разбор

- `pairs` — таблица соответствия: какой открывающей скобке отвечает каждая закрывающая.
- Для закрывающей скобки проверяем два условия сразу через `||`: стек пуст (нечего закрывать) **или** верх стека не совпадает с ожидаемой парой (неправильная вложенность/тип).
- Если проверка прошла — снимаем верхний элемент со стека (эта пара "закрыта").
- Финальная проверка `stk.empty()` нужна, чтобы отсечь случай `"((("` — все скобки открывающие, ни одна закрывающая проверка не сработала, но в конце в стеке остались незакрытые скобки.

### Пример

```
s = "{[()]}"

'{' -> push -> stack: {
'[' -> push -> stack: {, [
'(' -> push -> stack: {, [, (
')' -> top='(' == pairs[')']='(' -> pop -> stack: {, [
']' -> top='[' == pairs[']']='[' -> pop -> stack: {
'}' -> top='{' == pairs['}']='{' -> pop -> stack: (пусто)

Конец: stack.empty() == true -> valid
```

```
s = "([)]"

'(' -> push -> stack: (
'[' -> push -> stack: (, [
')' -> top='[' != pairs[')']='(' -> false (неправильная вложенность)
```

### Сложность

- Время: **O(n)** — один проход по строке.
- Память: **O(n)** — в худшем случае (все символы открывающие) стек хранит все `n` символов.

### Альтернатива без `unordered_map`

Можно class обойтись без хеш-таблицы, используя `switch` или прямое сравнение символов — чуть быстрее по константе за счёт отсутствия хеширования, но менее компактно:

### Частые вариации

- **Generate Parentheses** — сгенерировать все валидные комбинации `n` пар скобок (backtracking).
- **Longest Valid Parentheses** — найти длину самой длинной валидной подстроки (DP или стек с индексами).
- **Minimum Remove to Make Valid Parentheses** — убрать минимум скобок, чтобы строка стала валидной (стек с индексами для удаления).
