---
tags:
  - programming-language
  - cpp
  - containers
  - map
---
[[_cpp containers|<=]]

`std::unordered_map` - это ассоциативный контейнер, хранящий пары ключ-значение с уникальными ключами. В отличие от `std::map`, элементы не упорядочены, а организованы с помощью хеш-таблицы.

1. **Быстрый доступ**: Средняя сложность O(1) для операций вставки, удаления и поиска
2. **Уникальные ключи**: Каждый ключ может встречаться только один раз
3. **Хеш-таблица**: Использует хеш-функцию для организации данных
4. **Нет порядка**: Элементы не сортируются по ключам

- [[_cpp containers unordered_map - init|init]]
- [[_cpp containers unordered_map - addition|addition]]
- [[_cpp containers unordered_map - access|access]]
- [[_cpp containers unordered_map - deleting|deleting]]
- [[_cpp containers unordered_map - size|size]]
- [[_cpp containers unordered_map - setting|setting]]
