---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::swappable_with<T, U>`** из заголовка `<concepts>` проверяет, можно ли обменять два объекта **разных типов**, используя `using std::swap; swap(t, u);`.

## 📘 Что означает `std::swappable_with<T, U>`?

Типы `T` и `U` удовлетворяют концепту, если выражение:
```cpp
using std::swap;
swap(t, u);
```
корректно компилируется, где `t` имеет тип `T`, а `u` — тип `U`.

✅ Это позволяет обменивать **объекты разных, но совместимых типов**, например:
- `std::unique_ptr<Base>` и `std::unique_ptr<Derived>`
- Пользовательские типы с перегруженным `swap`

### Важные моменты

| Условие | Результат |
|--------|----------|
| `swap(t, u)` доступен (через ADL или `std::swap`) | ✅ `true` |
| Типы имеют общее выравнивание и размер | Но не гарантирует `swappable_with` |
| `T` и `U` — ссылки | Да, может работать |
| `T` и `U` нельзя сконструировать друг из друга | Часто → `false` |

> 💡 `std::swappable_with<T, U>` требует, чтобы:
> - `T` был lvalue,
> - `U` был lvalue,
> - Существовал `swap(t, u)` или `swap(u, t)`.

### Лучшая практика

| Совет                                                                  | Почему                  |
| ---------------------------------------------------------------------- | ----------------------- |
| Определяйте `swap` для своих типов в той же области                    | Чтобы работал ADL       |
| Делайте `swap` `noexcept`                                              | Для эффективности в STL |
| Используйте `using std::swap; swap(a, b);`                             | Идиоматический способ   |
| Предпочитайте `swappable_with` в шаблонах, работающих с разными типами | Гибкость                |

```cpp
#include <iostream>
#include <concepts>
#include <utility>

template<typename T, typename U>
requires std::swappable_with<T, U>
void exchange(T&& a, U&& b) {
    using std::swap;
    swap(a, b);
}

int main() {
    int a = 10;
    int b = 20;

    exchange(a, b);

    std::cout << a << " " << b << std::endl;
}
```

```
20 10
```
