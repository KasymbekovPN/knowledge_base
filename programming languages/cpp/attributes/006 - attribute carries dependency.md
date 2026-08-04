---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[carries_dependency]]` — редкий атрибут из C++11, связанный с моделью памяти `std::memory_order_consume` (сейчас практически не используется на практике, потому что `memory_order_consume` был признан слишком сложным для реализации и большинство компиляторов трактуют его как `memory_order_acquire`). Ставится на параметр функции или на саму функцию (для возвращаемого значения), сообщая компилятору, что зависимость данных (dependency chain) "переносится" через эту функцию из consume-операции, что в теории позволяет компилятору не вставлять более дорогой barrier для acquire-семантики, а полагаться на порядок зависимостей данных, который и так гарантирован на большинстве архитектур (кроме DEC Alpha).

```cpp
[[carries_dependency]] int* load_ptr(std::atomic<int*>& p) {
    return p.load(std::memory_order_consume);
}

void use([[carries_dependency]] int* ptr) {
    std::cout << *ptr;  // зависимость от ptr "перенесена" через параметр
}
```

На практике этот атрибут почти нигде не встретишь в реальном коде — `memory_order_consume` де-факто deprecated (в C++26 официально помечен как deprecated), и вместо него везде используют `memory_order_acquire`. Знать стоит скорее для полноты картины стандартных атрибутов, чем для использования.
