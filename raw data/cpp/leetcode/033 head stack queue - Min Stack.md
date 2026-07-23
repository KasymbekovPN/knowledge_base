[[raw data/cpp/interview/_|<=]]

## Min Stack

**Условие:** спроектировать стек, который в дополнение к обычным операциям (`push`, `pop`, `top`) поддерживает `getMin()` — получение минимального элемента во всём стеке, причём **все операции должны выполняться за O(1)**.

### Идея

Наивно — искать минимум линейным проходом при каждом вызове `getMin()`: O(n), не подходит. Идея решения: хранить **второй, вспомогательный стек**, который параллельно основному отслеживает "текущий минимум на момент каждого push". Тогда на вершине вспомогательного стека всегда лежит минимум для текущего состояния основного стека.

Ключевое наблюдение: при `pop()` из основного стека соответствующий элемент нужно убрать и из вспомогательного — тогда его вершина автоматически "откатится" к минимуму, который был актуален до этого push. Это работает, потому что оба стека растут и уменьшаются синхронно.

### Решение

```cpp
#pragma once  
  
#include <stack>  
  
namespace min_stack {  
  
    class MinStack {  
    public:  
        void push(int);  
        void pop();  
        int top() const;  
        int get_min() const;  
    private:  
        std::stack<int> data_;  
        std::stack<int> min_;  
    };  
    class MinStackOpt {  
    public:  
        void push(int);  
        void pop();  
        int top() const;  
        int get_min() const;  
    private:  
        std::stack<int> data_;  
        std::stack<std::pair<int, int>> min_;  
    };  
    void demo();  
}
```

```cpp
#include "min_stack.h"  
  
#include <iostream>  
#include <format>  
  
namespace min_stack {  
  
void MinStack::push(const int value) {  
    data_.push(value);  
  
    if (min_.empty() || value <= min_.top()) {  
        min_.push(value);  
    } else {  
        min_.push(min_.top());  
    }
}  
  
void MinStack::pop() {  
    data_.pop();  
    min_.pop();  
}  
  
int MinStack::top() const {  
    return min_.top();  
}  
  
int MinStack::get_min() const {  
    return min_.top();  
}  
  
void MinStackOpt::push(const int value) {  
    data_.push(value);  
    if (min_.empty() || value < min_.top().first) {  
        min_.push({value, 1});  
    } else {  
        ++min_.top().second;  
    }
}  
  
void MinStackOpt::pop() {  
    if (data_.top() == min_.top().first) {  
        if (--min_.top().second == 0) {  
            min_.pop();  
        }    
    }    
    data_.pop();  
}  
  
int MinStackOpt::top() const {  
    return min_.top().first;  
}  
  
int MinStackOpt::get_min() const {  
    return min_.top().first;  
}  
  
void demo() {  
    auto stk = MinStack();  
    stk.push(-2);  
    stk.push(0);  
    stk.push(-3);  
  
    std::cout << std::format("STK min: {}\n", stk.get_min());  
    stk.pop();  
    std::cout << std::format("STK min: {}\n", stk.get_min());  
  
    auto stk_opt = MinStackOpt();  
    stk_opt.push(-2);  
    stk_opt.push(0);  
    stk_opt.push(-3);  
  
    std::cout << std::format("STK opt min: {}\n", stk_opt.get_min());  
    stk_opt.pop();  
    std::cout << std::format("STK opt min: {}\n", stk_opt.get_min());  
}  
  
}
```

### Разбор

- `data_` — обычный стек с реальными значениями.
- `minStack_` растёт **синхронно** с `data_`: при каждом `push` в `data_` в `minStack_` тоже добавляется ровно один элемент — либо новое значение (если оно меньше или равно текущему минимуму), либо повторно текущий минимум (если новое значение больше). За счёт этого дублирования на вершине `minStack_` **всегда** лежит минимум по всем элементам, которые сейчас есть в `data_`.
- При `pop()` снимаем верхний элемент с обоих стеков одновременно — это гарантирует, что `minStack_.top()` снова покажет корректный минимум для оставшихся элементов, "откатившись" к состоянию до последнего push.
- `val <= minStack_.top()` (не строгое `<`) — важно при дубликатах минимального значения: если положить в стек то же самое минимальное значение ещё раз, а потом снять его один раз через `pop()`, минимум не должен "потеряться" преждевременно — синхронный рост двух стеков с `<=` это корректно обрабатывает.

### Пример

```
push(-2): data=[-2], minStack: val=-2, пусто -> push(-2); minStack=[-2]
push(0):  data=[-2,0], minStack: val=0, 0<=-2? нет -> push(top=-2); minStack=[-2,-2]
push(-3): data=[-2,0,-3], minStack: val=-3, -3<=-2? да -> push(-3); minStack=[-2,-2,-3]

getMin() -> minStack.top() = -3

pop(): data=[-2,0], minStack=[-2,-2]  (сняли по одному с каждого)
getMin() -> minStack.top() = -2

top() -> data.top() = 0
getMin() -> -2
```

### Сложность

- Время: **O(1)** для всех операций (`push`, `pop`, `top`, `getMin`).
- Память: **O(n)** — два стека одинакового размера вместо одного (константа × 2, но асимптотика та же).

### Альтернатива: экономия памяти через хранение только "смен минимума"

Вместо дублирования минимума на каждый push, можно класть во вспомогательный стек новое минимальное значение **только когда оно реально меняется**, плюс хранить, сколько элементов подряд имели этот минимум (или просто не добавлять дубликаты, а только пары `(значение, счётчик повторов)`). Это экономит память в случаях, когда новые минимумы устанавливаются редко, но усложняет код — обычно на собеседовании достаточно показать базовое решение с синхронными стеками, и только по запросу — обсудить эту оптимизацию.

### Частые вариации

- **Max Stack** — та же идея, но для максимума; часто требует ещё и операцию `popMax()` (удалить максимальный элемент, не обязательно с вершины) — тогда нужна более сложная структура (например, два `std::multiset` или дважды связный список + hash map).
- **Design a Stack With Increment Operation** — операция "увеличить нижние k элементов на val" — решается через отложенное суммирование (lazy propagation) без пересчёта всех элементов сразу.
- **Implement Queue using Stacks** / **Implement Stack using Queues** — классические задачи на построение одной структуры поверх другой, схожая тема "две вспомогательные структуры вместо одной".
