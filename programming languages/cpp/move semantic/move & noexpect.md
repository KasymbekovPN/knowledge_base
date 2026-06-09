---
tags:
  - programming-language
  - cpp
  - move-semantic
---
[[programming languages/cpp/move semantic/_|<=]]

В C++ ключевое слово `noexcept` играет **очень важную роль** при использовании **move семантики**, особенно при работе с **стандартными контейнерами STL** и **алгоритмами**.

`noexcept` — это спецификатор, который указывает компилятору, что функция **не бросает исключений**.

Если функция, помеченная как `noexcept`, всё же выбросит исключение — вызовется `std::terminate()`.

> **STL (например, `std::vector`) использует `std::is_nothrow_move_constructible` для выбора между копированием и перемещением.**

Когда вы увеличиваете размер `std::vector`, он может пересоздавать внутренний массив и **переносить элементы на новое место**.

- Если **move конструктор не `noexcept`**, то:
  - `std::vector` будет **копировать** элементы вместо перемещения,
  - Это **снижает производительность**.
- Если move конструктор **помечен как `noexcept`**, то:
  - `std::vector` будет **безопасно использовать move**, чтобы расширить массив.

### Как проверить, является ли move безопасным?

Используйте `std::is_nothrow_move_constructible_v<T>` из `<type_traits>`:

```cpp
#include <type_traits>
#include <vector>

static_assert(std::is_nothrow_move_constructible_v<std::vector<int>>, "Vector should be nothrow move constructible");
```

Это гарантирует, что STL будет использовать move вместо copy.

> ✅ ВСЕГДА помечайте move методы как `noexcept`, если они действительно не могут бросить исключения.
