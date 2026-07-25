[[raw data/cpp/interview/_|<=]]

## Trapping Rain Water

**Условие:** дан массив `height`, где `height[i]` — высота столбика в позиции `i`. После дождя вода задерживается между столбиками. Посчитать суммарный объём воды, который удержится.

### Идея

Количество воды над позицией `i` определяется формулой:

`water[i] = max(0, min(maxLeft[i], maxRight[i]) - height[i])`

где `maxLeft[i]` — максимальная высота среди всех столбиков **слева** от `i` (включая сам `i`), `maxRight[i]` — максимальная высота **справа** от `i` (включая сам `i`). Вода над позицией `i` ограничена **меньшим** из двух максимумов (более высокая "стена" с одной стороны не помогает, если с другой стороны стена ниже — вода перельётся через неё).

```cpp
#include "trapping_rain_water.h"  
  
#include <vector>  
#include <algorithm>  
#include <iostream>  
#include <format>  
#include <concepts>  
#include <ostream>  
  
namespace trapping_rain_water {  
  
#define PRINT  
  
template <typename T>  
concept Streamable = requires(std::ostream& os, const T& value) {  
    { os << value } -> std::same_as<std::ostream&>;  
};  
  
template <Streamable T>  
static void print(const std::vector<T>& vec, const std::string& label) {  
#ifdef PRINT  
    std::cout << std::format("{}: ", label);  
    for (const auto& elem : vec) {  
        std::cout << elem << ' ';  
    }    std::cout << '\n';  
#endif  
}  
  
int trap(const std::vector<int>& heights) {  
    const int N{static_cast<int>(heights.size())};  
    if (N == 0) return 0;  
  
    std::vector<int> max_left(N), max_right(N);  
  
    max_left[0] = heights[0];  
    for (int i{1}; i < N; ++i) {  
        max_left[i] = std::max(max_left[i - 1], heights[i]);  
    }  
    max_right[N - 1] = heights[N - 1];  
    for (int i{N-2}; i >= 0; --i) {  
        max_right[i] = std::max(max_right[i + 1], heights[i]);  
    }  
    int total{};  
    for (int i{}; i < N; ++i) {  
        total += std::min(max_left[i], max_right[i]) - heights[i];  
    }  
    print(heights, "heights");  
    print(max_left, "max_left");  
    print(max_right, "max_right");  
  
    return total;  
}  
  
int trap_two_pointer(const std::vector<int>& heights) {  
    const int N{static_cast<int>(heights.size())};  
    if (N == 0) return 0;  
  
    int left{}, right{N-1};  
    int left_max{}, right_max{};  
    int total{};  
  
    while (left < right) {  
        if (heights[left] < heights[right]) {  
            left_max = std::max(left_max, heights[left]);  
            total += left_max - heights[left];  
            ++left;  
        } else {  
            right_max = std::max(right_max, heights[right]);  
            total += right_max - heights[right];  
            --right;  
        }    
    }  
    return total;  
}  
  
void demo() {  
    const std::vector<int> HEIGHTS{0,1,0,2,1,0,1,3,2,1,2,1};  
  
    std::cout << std::format("TRAP {}\n", trap(HEIGHTS));  
    std::cout << std::format("TRAP 2P {}\n", trap_two_pointer(HEIGHTS));  
}  
}
```


### Решение 1: префиксные и суффиксные максимумы (наглядно, O(n) память)

**Разбор:** `maxLeft[i]` считается одним проходом слева направо — накопленный максимум. `maxRight[i]` — аналогично, но справа налево. Затем для каждой позиции вода — разница между "потолком" (меньшим из двух максимумов) и текущей высотой столбика; если разница отрицательна (столбик сам выше потолка), значит воды здесь нет — но по формуле выше `min(maxLeft[i], maxRight[i]) >= height[i]` всегда (так как оба максимума учитывают и сам `height[i]`), поэтому `std::min(...) - height[i]` никогда не отрицательно и явный `max(0, ...)` не требуется.

Время: **O(n)** — три линейных прохода. Память: **O(n)** — под `maxLeft` и `maxRight`.

### Решение 2: два указателя (оптимально по памяти, O(1))

**Идея:** вместо того чтобы заранее считать полные массивы `maxLeft`/`maxRight`, поддерживаем их "на лету" через два указателя, движущихся навстречу друг другу. Ключевое наблюдение: если `leftMax < rightMax`, то вода над позицией `left` определяется **только** `leftMax` — независимо от точного значения `rightMax` (мы уже знаем, что справа есть стена не ниже `rightMax > leftMax`, значит именно `leftMax` — узкое место для позиции `left`). Это позволяет решать задачу за один проход, двигая всегда указатель с меньшим текущим максимумом.

### Разбор

- `leftMax`/`rightMax` — максимумы, накопленные указателями `left`/`right` **к текущему моменту** (не полные `maxLeft[i]`/`maxRight[i]` из первого решения, а только частично известные значения).
- Условие `height[left] < height[right]` определяет, какой указатель двигать. Обоснование корректности: если `height[left] < height[right]`, то и `leftMax <= height[right] <= rightMax_реальный` (так как `rightMax` в конечном счёте включает `height[right]`, а возможно и что-то ещё большее правее). Значит для позиции `left` потолок точно определяется `leftMax`, независимо от того, что там точно с `rightMax` — можно безопасно посчитать воду для `left` прямо сейчас и сдвинуть указатель.
- `leftMax - height[left]` — вода над текущей позицией `left`, аналогично для `right`.
- `leftMax`/`rightMax` обновляются **до** вычисления воды на этом шаге — если текущий столбик выше текущего максимума, он сам становится новым максимумом (и тогда вода над ним — 0, так как `leftMax - height[left] = 0`).

### Пример

```
height = [0,1,0,2,1,0,1,3,2,1,2,1]

left=0,right=11: height[0]=0<height[11]=1 -> leftMax=max(0,0)=0; total+=0-0=0; left=1
left=1,right=11: height[1]=1<height[11]=1? нет(равны, идём в else) -> rightMax=max(0,1)=1; total+=1-1=0; right=10
left=1,right=10: height[1]=1<height[10]=2 -> leftMax=max(0,1)=1; total+=1-1=0; left=2
left=2,right=10: height[2]=0<height[10]=2 -> leftMax=max(1,0)=1; total+=1-0=1; left=3   (total=1)
left=3,right=10: height[3]=2<height[10]=2? нет -> rightMax=max(1,2)=2; total+=2-2=0; right=9
left=3,right=9: height[3]=2<height[9]=1? нет -> rightMax=max(2,1)=2; total+=2-1=1; right=8  (total=2)
left=3,right=8: height[3]=2<height[8]=2? нет -> rightMax=max(2,2)=2; total+=2-2=0; right=7
left=3,right=7: height[3]=2<height[7]=3 -> leftMax=max(1,2)=2; total+=2-2=0; left=4
left=4,right=7: height[4]=1<height[7]=3 -> leftMax=max(2,1)=2; total+=2-1=1; left=5  (total=3)
left=5,right=7: height[5]=0<height[7]=3 -> leftMax=max(2,0)=2; total+=2-0=2; left=6  (total=5)
left=6,right=7: height[6]=1<height[7]=3 -> leftMax=max(2,1)=2; total+=2-1=1; left=7  (total=6)
left=7,right=7: цикл завершён (left<right ложно)

Результат: 6
```

### Сложность

- Время: **O(n)** — один проход, каждый указатель проходит массив ровно один раз суммарно.
- Память: **O(1)** — в отличие от первого решения, никаких вспомогательных массивов.

### Сравнение подходов

- **Префиксные/суффиксные максимумы** — интуитивно понятнее, легче объяснить и вывести на собеседовании с нуля, но требует O(n) доп. памяти.
- **Два указателя** — оптимален по памяти (O(1)), но корректность требует более тонкого обоснования ("почему можно двигать именно этот указатель"), обычно предлагается как оптимизация после первого решения.

### Частые вариации

- **Trapping Rain Water II** — 2D-версия (сетка высот вместо одномерного массива) — решается через min-heap (приоритетную очередь) и обход "снаружи внутрь", начиная с границ сетки, аналогично Dijkstra по концепции.
- **Container With Most Water** (уже разобрана) — похожая механика двух указателей, но там ищем **одну пару** стенок с максимальной площадью, а не суммарный объём воды над **каждой** позицией.
- **Product of Array Except Self** (уже разобрана) — структурно похожий приём "префиксные + суффиксные агрегаты", но там произведение, а не максимум.
