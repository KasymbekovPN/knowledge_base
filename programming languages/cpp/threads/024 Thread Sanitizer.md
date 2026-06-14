---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## -fsanitize=thread (TSan)

Флаг компилятора, включающий **Thread Sanitizer** — инструмент динамического анализа, который обнаруживает гонки данных (data races) во время выполнения программы.

```bash
clang++ -std=c++23 -fsanitize=thread -g main.cpp -o main
```

`-g` рекомендуется вместе — для читаемых stack trace с номерами строк.

### Что обнаруживает

```cpp
int counter = 0;

std::thread t0([&]() { ++counter; }); // гонка
std::thread t1([&]() { ++counter; }); // гонка

t0.join(); t1.join();
```

TSan выведет:

```
WARNING: ThreadSanitizer: data race
  Write of size 4 at 0x... by thread T2:
    #0 main.cpp:5
  Previous write of size 4 at 0x... by thread T1:
    #0 main.cpp:4
```

### Как работает

Инструментирует **каждое обращение к памяти** во время компиляции, а в runtime отслеживает какой поток и когда обратился к каждой ячейке. Замедление программы — примерно **5–15x**.

### Ограничения

- Только **Linux / macOS** — на Windows не поддерживается
- Не находит гонки которые не произошли во время конкретного запуска
- Ложные срабатывания при использовании нестандартных примитивов синхронизации

### Другие sanitizer-ы

| Флаг                   | Что ищет                            |
| ---------------------- | ----------------------------------- |
| `-fsanitize=thread`    | гонки данных                        |
| `-fsanitize=address`   | выход за границы, use-after-free    |
| `-fsanitize=memory`    | чтение неинициализированной памяти  |
| `-fsanitize=undefined` | UB (переполнение, null deref и др.) |

## TSan на Windows не поддерживается ни clang, ни MSVC. Есть три варианта

### 1. WSL2 (рекомендуется)

Запустить в Linux-окружении прямо из Windows:

```bash
# в терминале WSL2
clang++ -std=c++23 -fsanitize=thread -g demo.cpp -o demo
./demo
```

WSL2 устанавливается через: `wsl --install` в PowerShell.

### 2. Visual Studio — AddressSanitizer

TSan в VS не поддерживается, но ASan есть:

```bash
clang++ -fsanitize=address -g demo.cpp -o demo
```

Для гонок данных это не поможет, но для других ошибок памяти — да.

### 3. Для конкретно этого кода — просто запустить

Цель примера с `seq_cst` — проверить что `z >= 1`. Если компилируется и `z` всегда `>= 1` — гарантии `seq_cst` работают. Гонки данных в этом коде нет — `z` атомарный, `x` и `y` атомарные.

```powershell
clang++ -std=c++23 demo.cpp -o demo.exe
.\demo.exe
```

Смотреть на значение `z` — должно быть `1` или `2`.
