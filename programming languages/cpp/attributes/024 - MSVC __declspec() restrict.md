---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__declspec(restrict)` — применяется к объявлению функции, возвращающей указатель, и говорит компилятору, что этот указатель не алиасится ни с каким другим указателем, видимым в программе на момент вызова — то есть функция ведёт себя как `malloc`: возвращает "свежую", ни с чем не пересекающуюся область памяти. Это позволяет компилятору агрессивнее оптимизировать код, который работает с результатом такой функции, не опасаясь, что запись через этот указатель могла повлиять на данные, доступные через другие указатели.

```cpp
__declspec(restrict) void* customAlloc(size_t size);

void useIt() {
    int* a = static_cast<int*>(customAlloc(sizeof(int) * 10));
    int* b = static_cast<int*>(customAlloc(sizeof(int) * 10));
    // компилятор знает: a и b точно не пересекаются между собой
    // и не пересекаются ни с чем ранее существовавшим — можно
    // переупорядочивать/кэшировать обращения к *a и *b агрессивнее
}
```

Важно не путать с двумя похожими, но разными вещами:

- **C99/C11 `restrict`** (в C++ — расширение `__restrict`/`__restrict__` у большинства компиляторов, включая MSVC) — применяется к _параметру-указателю_, а не к возвращаемому значению, и говорит "через этот указатель обращаются только так, никакой другой указатель на тот же объект в этой функции не используется". `__declspec(restrict)` решает симметричную, но другую задачу — про то, что возвращает функция, а не про то, что она принимает.
- **GCC/Clang-эквивалент** — `__attribute__((malloc))`: семантически то же самое (возвращаемый указатель не алиасится и, что важно, ещё и подразумевает, что предыдущее содержимое памяти по этому адресу "не имеет значения" для оптимизатора, как для настоящего `malloc`). Стандартного `[[...]]`-аналога у обоих нет.

Пример корректного использования — обёртка над кастомным аллокатором в высокопроизводительном коде:

```cpp
#if defined(_MSC_VER)
  #define ALLOC_RETURN __declspec(restrict)
#elif defined(__GNUC__)
  #define ALLOC_RETURN __attribute__((malloc))
#else
  #define ALLOC_RETURN
#endif

ALLOC_RETURN void* poolAlloc(size_t size);
```

Осторожность та же, что и с `pure`/`const` из GCC-атрибутов, обсуждавшихся раньше: это обещание компилятору, а не проверяемое условие. Если функция на самом деле может вернуть указатель, пересекающийся с чем-то ещё живым (например, переиспользует буфер из пула, который ещё не был явно освобождён вызывающим кодом), пометка `restrict` — UB, компилятор может сделать неверные предположения об отсутствии алиасинга и сгенерировать код, ломающий программу непредсказуемым образом.

### Пример
```cpp
#include <format>  
#include <iostream>  
#include <unordered_set>  
  
#if defined(_MSC_VER)  
    #define ALLOC_RETURN __declspec(restrict)  
#elif defined(__GNUC__)  
    #define ALLOC_RETURN __attribute__((malloc))  
#else  
    #define ALLOC_RETURN  
#endif  
  
namespace {  
  
    class Point {  
    public:  
        static Point* instance(const double x, const double y) {  
            const auto p = new Point(x, y);  
            points.insert(p);  
  
            return p;  
        }  
        
        static void freeAll() {  
            const auto copy = points;  
            points.clear();  
  
            for (const auto p : copy) {  
                delete p;  
            }        
        }  

        Point() = delete;  
  
        double x, y;  
    private:  
        Point(const double x, const double y) : x(x), y(y) {}  
  
        inline static std::unordered_set<Point*> points;  
    };
  
    ALLOC_RETURN void* pointAlloc() {  
        return Point::instance(0, 0);  
    }}  
  
int main() {  
    const auto p = static_cast<Point*>(pointAlloc());  
    std::cout << std::format("({}, {})\n", p->x, p->y);  
    Point::freeAll();  
  
    return 0;  
}
```
