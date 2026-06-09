---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers - unordered_map|<=]]

| Метод              | Вставка нового | Обновление существующего | Возвращаемое значение  | Эффективность |
| ------------------ | -------------- | ------------------------ | ---------------------- | ------------- |
| `operator[]`       | Да             | Да                       | Ссылка на значение     | Средняя       |
| `insert`           | Да             | Нет                      | `pair<iterator, bool>` | Средняя       |
| `emplace`          | Да             | Нет                      | `pair<iterator, bool>` | Высокая       |
| `try_emplace`      | Да             | Нет                      | `pair<iterator, bool>` | Очень высокая |
| `insert_or_assign` | Да             | Да                       | `pair<iterator, bool>` | Высокая       |

## Рекомендации

1. Для простых типов используйте `operator[]` или `insert`
2. Для сложных типов предпочитайте `emplace` и `try_emplace`
3. Для обновления значений используйте `insert_or_assign` или `operator[]`
4. При массовой вставке сначала резервируйте место (`reserve()`)

## Особенности производительности

1. `operator[]`:
   - Дважды ищет ключ (при проверке и вставке)
   - Создает временный объект значения по умолчанию при доступе

2. `try_emplace`:
   - Оптимизирован для случая, когда ключ уже существует
   - Не создает временных объектов

3. `insert_or_assign`:
   - Эквивалентен `operator[]`, но с дополнительной информацией о результате


- [[cpp containers unordered_map addition - insert|insert]]
- [[cpp containers unordered_map addition - subscript|subscript]]
- [[cpp containers unordered_map addition - emplace|emplace]]
- [[cpp containers unordered_map addition - try_emplace|try_emplace]]
- [[cpp containers unordered_map addition - insert_or_assign|insert_or_assign]]