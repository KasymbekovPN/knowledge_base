---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::aligned_storage_t<Len, Align>`** из заголовка `<type_traits>` предоставляет способ получить **выровненный блок памяти фиксированного размера**, подходящий для размещения объекта заданного размера и выравнивания.

### Что делает `std::aligned_storage_t<Len, Align>`?

Он определяет тип, который:
- Имеет размер как минимум `Len`,
- Выровнен по границе `Align` (или по умолчанию — наилучшее выравнивание),
- Может использоваться как "сырая" память для `placement new`.

✅ Это полезно при реализации:
- Собственных контейнеров,
- Объектных пулов,
- Алокаторов,
- `variant`-подобных типов.

## ⚠️ Важно: Устарело в C++23

Начиная с **C++23**, `std::aligned_storage_t` и `std::aligned_storage` объявлены **устаревшими (deprecated)**.

👉 Современная альтернатива:
```cpp
alignas(Align) std::byte storage[Len];
```
или просто использовать `std::aligned_alloc` / `operator new` с выравниванием.

## ⚠️ Ограничения и важные моменты

| Особенность | Пояснение |
|------------|-----------|
| `Len` и `Align` должны быть константами времени компиляции | Не работает с переменными |
| `Align` должен быть степенью двойки | Иначе — неопределённое поведение |
| Не инициализирует память | Только сырое хранилище |
| Требует ручной вызов `~T()` | RAII обязателен |

## ✅ Лучшая практика

| Совет | Почему |
|------|--------|
| Используйте `aligned_storage_t` только в C++11–C++20 | В C++23+ он deprecated |
| Предпочитайте `alignas(N) std::byte[N]` | Современный, читаемый способ |
| Всегда вызывайте деструктор вручную | Безопасность ресурсов |
| Инкапсулируйте в классе | Как `variant`, `optional` |
| Избегайте прямого управления памятью без нужды | Используйте `std::unique_ptr`, `std::vector` |


```cpp
#include <iostream>
#include <type_traits>
#include <new>

struct V3 {
    float x, y, z;
    V3(float _x, float _y, float _z):
        x{_x},
        y{_y},
        z{_z} {}
};

using Storage = std::aligned_storage_t<sizeof(V3), alignof(V3)>;

int main() {
    alignas(Storage) char buffer[sizeof(Storage)];
    Storage* storage = reinterpret_cast<Storage*>(buffer);

    V3* vec = new(storage) V3(1.0f, 2.0f, 3.0f);

    std::cout
        << "{" << vec->x
        << ", " << vec->y
        << ", " << vec->z
        << "}" << std::endl;

    vec->~V3();

    return 0;
}
```

> ✅ Память выровнена правильно → безопасное создание.

```
{1, 2, 3}
```
