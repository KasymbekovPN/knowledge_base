---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers - unordered_map|<=]]

## Особенности удаления

1. **Сложность операций**:
   - Удаление по ключу: средняя O(1), худший случай O(n)
   - Удаление по итератору: средняя O(1)
   - Удаление диапазона: O(m), где m - количество удаляемых элементов
   - `clear()`: O(n)

2. **Инвалидация итераторов**:
   - Удаление элемента делает недействительными итераторы на удаленный элемент
   - Другие итераторы обычно остаются валидными

3. **Возвращаемые значения**:
   - `erase(key)` возвращает количество удаленных элементов (0 или 1)
   - `erase(iterator)` возвращает итератор на следующий элемент (начиная с C++11)

- [[cpp containers unordered_map delete - by key|by key]]
- [[cpp containers unordered_map delete - by iter|by iter]]
- [[cpp containers unordered_map delete - by range|by range]]
- [[cpp containers unordered_map delete - clear|clear]]
- [[cpp containers unordered_map delete - by condition|by condition]]