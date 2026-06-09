---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers - unordered_map|<=]]

## Сравнение методов доступа

| Метод        | Создает элемент если нет | Бросает исключение | Возвращаемое значение | C++ версия |
| ------------ | ------------------------ | ------------------ | --------------------- | ---------- |
| `operator[]` | Да                       | Нет                | Ссылка на значение    | C++98      |
| `at()`       | Нет                      | Да                 | Ссылка на значение    | C++11      |
| `find()`     | Нет                      | Нет                | Итератор              | C++98      |
| `contains()` | Нет                      | Нет                | bool                  | C++20      |

## Особенности производительности

1. Все методы (`[]`, `at`, `find`, `contains`) имеют среднюю сложность O(1)
2. В худшем случае (при коллизиях) сложность может достигать O(n)
3. `operator[]` менее эффективен при доступе к несуществующим ключам (создает элемент)

## Рекомендации

1. Для **чтения** используйте `find()` или `contains()` + `operator[]`
2. Для **безопасного доступа** используйте `at()` (с обработкой исключений)
3. Для **записи/модификации** используйте `operator[]` или `insert_or_assign`
4. В C++20+ предпочитайте `contains()` для проверки наличия ключа

- [[cpp containers unordered_map access - subscript|subscript]]
- [[cpp containers unordered_map access - at|at]]
- [[cpp containers unordered_map access - find|find]]
- [[cpp containers unordered_map access - contains|contains]]