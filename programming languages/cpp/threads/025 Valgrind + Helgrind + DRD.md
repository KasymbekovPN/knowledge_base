---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## Valgrind + Helgrind/DRD

**Valgrind** — фреймворк для динамического анализа программ на Linux/macOS. Запускает программу в виртуальной машине и инструментирует каждую инструкцию.

**Helgrind** и **DRD** — два инструмента внутри Valgrind для обнаружения гонок данных.

### Запуск

```bash
# Helgrind
valgrind --tool=helgrind ./demo

# DRD
valgrind --tool=drd ./demo
```

### Helgrind

Ищет нарушения **happens-before** отношений между потоками:

```
==12345== Possible data race during read of size 4 at 0x...
==12345==    at 0x...: main (demo.cpp:5)
==12345==  This conflicts with a previous write of size 4 by thread #2
==12345==    at 0x...: main (demo.cpp:4)
```

Отслеживает:

- гонки данных
- неправильный порядок захвата мьютексов (потенциальный дедлок)
- `pthread` API ошибки

### DRD (Data Race Detector)

Аналог Helgrind, но другой алгоритм — **векторные часы** на уровне каждого потока. Потребляет меньше памяти, быстрее на большом числе потоков.

```bash
valgrind --tool=drd --check-stack-var=yes ./demo
```

### Helgrind vs DRD vs TSan

| |Helgrind|DRD|TSan|
|---|---|---|---|
|Алгоритм|happens-before|векторные часы|векторные часы|
|Замедление|~20–50x|~10–20x|~5–15x|
|Память|высокое потребление|меньше|среднее|
|Платформа|Linux/macOS|Linux/macOS|Linux/macOS|
|Windows|нет|нет|нет|

### На практике для Windows

Единственный путь — **WSL2**:

```bash
# установить valgrind в WSL2
sudo apt install valgrind

# скомпилировать с отладочными символами
g++ -std=c++23 -g demo.cpp -o demo

# запустить
valgrind --tool=helgrind ./demo
```