---
tags:
  - programming-language
  - cpp
  - containers
  - priority_queue
---
[[_cpp containers|<=]]

В C++ `std::priority_queue` — это адаптер контейнера, который предоставляет интерфейс очереди с приоритетом. Элементы в `std::priority_queue` упорядочены таким образом, что элемент с наивысшим приоритетом всегда находится на вершине. По умолчанию используется контейнер `std::vector` и компаратор `std::less`, что делает `std::priority_queue` максимальной кучей (наибольший элемент на вершине).

- [[_cpp containers priority_queue - init|init]]
- [[_cpp containers priority_queue - addition|addition]]
- [[_cpp containers priority_queue - removing|removing]]
- [[_cpp containers priority_queue - access|access]]
- [[_cpp containers priority_queue - size|size]]
