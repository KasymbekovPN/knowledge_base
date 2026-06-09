---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/_|<=]]

Функция `std::sample` из заголовочного файла `<algorithm>` используется для **выбора случайной подвыборки элементов из диапазона**, сохраняя их порядок.

> ✅ Это удобно, когда вам нужно:
> - Выбрать случайные элементы из контейнера,
> - Реализовать случайную выборку без повторений,
> - Работать с большими наборами данных,
> - Создать тестовые данные или реализовать машинное обучение.

### Синтаксис

```cpp
#include <algorithm>
#include <random>

template<class PopulationIterator, class SampleIterator, class Distance, class URNG>
SampleIterator sample(PopulationIterator first, PopulationIterator last,
                      SampleIterator out, Distance n,
                      URNG&& g);
```

- `first`, `last` — итераторы на исходный "набор данных"
- `out` — куда записывать результаты
- `n` — сколько элементов выбрать
- `g` — генератор случайных чисел (`UniformRandomBitGenerator`)
- Возвращает итератор `out + actual_n`, где `actual_n = min(n, distance(first, last))`

### Важные моменты

| Особенность                               | Объяснение                                       |
| ----------------------------------------- | ------------------------------------------------ |
| Элементы выбираются **без повторений**    | Каждый элемент может быть выбран только один раз |
| Порядок элементов сохраняется             | Как в оригинальном диапазоне                     |
| Требуется генератор случайных чисел       | Например, `std::mt19937`                         |
| Если `n > size`, берётся `size` элементов | Безопасно                                        |
| Не модифицирует исходный диапазон         | Только читает                                    |

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

int main() {
    const vector<int> source {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    vector<int> sample_data (3);

    sample(
        source.begin(),
        source.end(),
        sample_data.begin(),
        sample_data.size(),
        mt19937(random_device()())
    );

    cout << "{";
    for(auto& item: sample_data) {
        cout << " " << item;
    }
    cout << " }" << endl;

    return 0;
}
```

```
{ 2 5 8 }
```
