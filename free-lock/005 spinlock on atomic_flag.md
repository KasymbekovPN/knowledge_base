
Реализация Spinlock на базе `std::atomic_flag` — классическая задача низкоуровневой синхронизации. Ниже представлена корректная реализация с учётом современных стандартов C++ (включая C++20 `test()`) и оптимизацией кэш-линий (алгоритм TTAS — Test and Test-and-Set). Также приведена инструкция по профилированию с помощью `perf` в Docker-контейнере.

---

## ЧАСТЬ 1. Реализация Spinlock на C++

Для максимальной производительности мы используем паттерн TTAS. В обычном цикле `while(flag.test_and_set())` ядро процессора на каждой итерации запрашивает кэш-линию в эксклюзивное состояние (Exclusive) для записи, что при высокой конкуренции (contention) порождает колоссальный трафик когерентности кэшей. [4, 5]

Паттерн TTAS сначала проверяет флаг в режиме «только чтение» (`test()`), позволяя кэш-линии оставаться в разделяемом состоянии (Shared) у всех ядер, пока лок занят. Также мы вставляем ассемблерную инструкцию `pause` (`_mm_pause()`), которая оптимизирует энергопотребление конвейера процессора при холостом цикле и предотвращает спекулятивное выполнение. [4, 6, 7]

```cpp
#include <iostream>
#include <print>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

#if defined(__x86_64__) || defined(_M_AMD64)
#include <x86intrin.h>
#define SPINLOCK_PAUSE() _mm_pause()
#else
#define SPINLOCK_PAUSE()
#endif

namespace {

    class SpinLock {
        // По стандарту изначально false
        std::atomic_flag flag = ATOMIC_FLAG_INIT;

    public:
        void lock() noexcept {
            for (;;) {
                // Оптимизация TTAS: крутимся в локальном кэше в режиме Read-Only,
                // пока флаг занят (C++20 предоставляет метод .test())
                if (!flag.test(std::memory_order_relaxed)) {
                    // Пытаемся захватить лок атомарной записью
                    if (!flag.test_and_set(std::memory_order_acquire)) return; // Успешно захватили
                    SPINLOCK_PAUSE();
                }
            }
        }

        void unlock() noexcept {
            // Освобождаем с семантикой Release
            flag.clear(std::memory_order_release);
        }
    };

    constexpr int NUM_THREADS{8};
    constexpr int ITERATIONS{1'000'000};
    long long global_counter{0};

    SpinLock spin;

    void worker() {
        for (int i{}; i < ITERATIONS; ++i) {
            spin.lock();
            global_counter++;
            spin.unlock();
        }
    }

}

int main() {
    std::vector<std::thread> threads;
    const auto start{std::chrono::high_resolution_clock::now()};

    for (int i{}; i< NUM_THREADS; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) thread.join();

    const auto end{std::chrono::high_resolution_clock::now()};
    std::chrono::duration<double, std::milli> elapsed{end - start};

    std::println(
        std::cout,
        "Counter: {}\nElapsed time: {} ms",
        global_counter,
        elapsed.count());

    return 0;
}

```

## ЧАСТЬ 2. Замер под контейнером с помощью perf

По умолчанию подсистема производительности ядра Linux (`perf`) заблокирована внутри Docker-контейнеров из соображений безопасности. Чтобы запустить профилирование, контейнеру необходимы специфические привилегии ядра.

## Шаг 1. Dockerfile для сборки окружения

Создайте `Dockerfile-fifo` в одной папке с `main.cpp`:

```dockerfile
FROM ubuntu:24.04

# Установка компилятора и утилит профилирования
RUN apt-get update && apt-get install -y \
    build-essential \
    linux-tools-common \
    linux-tools-generic \
    linux-tools-$(uname -r) \
    && rm -rf /var/list/apt/lists/*

WORKDIR /app
COPY main.cpp .

# Компилируем с флагами оптимизации и отладочными символами (-g для perf)
RUN g++ -O3 -g -std=c++20 main.cpp -o benchmark

CMD ["./benchmark"]
```

## Шаг 2. Запуск контейнера с правами для perf

Чтобы `perf` имел доступ к аппаратным счетчикам процессора, контейнер нужно запустить с флагом `--cap-add=SYS_ADMIN` (или `--privileged`). Также необходимо временно ослабить ограничения `perf_event_paranoid` на хост-машине.

Выполните на хост-системе (Linux / wsl):

```bash
# Разрешаем сбор CPU-профилей всем пользователям (0 или -1)
sudo sysctl -w kernel.perf_event_paranoid=-1
```

Сборка и интерактивный запуск контейнера:

```bash
docker build -t spinlock-perf .
# Запускаем контейнер в интерактивном режиме с нужными правами
docker run --rm -it --cap-add=SYS_ADMIN spinlock-perf /bin/bash
```

## Шаг 3. Профилирование внутри контейнера

Вы окажетесь внутри терминала запущенного Docker-контейнера. Запустите сбор статистики `perf`:

```bash
# 1. Запись профиля во время выполнения программы
perf record -g ./benchmark

# или
ls -d /usr/lib/linux-tools/*/
/usr/lib/linux-tools/6.8.0-139-generic/perf record -e cpu-clock -g ./benchmark

# 2. Просмотр отчета прямо в терминале
perf report

# или
/usr/lib/linux-tools/6.8.0-139-generic/perf report

```

## Что мы увидим в отчете `perf report`?

Интерактивный интерфейс `perf` покажет дерево вызовов («горячие» точки):

1. Около 80-90% процессорного времени будет сфокусировано внутри метода `Spinlock::lock()`.
2. Если раскрыть ассемблерный код внутри `perf report` (нажав клавишу `a` на строке метода), вы увидите, что основная масса циклов CPU «сгорает» на инструкциях `pause` и `test` (чтение флага), а не на инструкции `lock bts` / `lock xchg` (которая генерируется для `test_and_set`), что подтверждает эффективность паттерна TTAS. [4]

Если вы хотите углубиться в оптимизацию, дайте знать:

- Какая у вас архитектура процессора (x86_64 или ARM/Apple Silicon)?
- Хотите ли вы сравнить этот `Spinlock` со стандартным `std::mutex` или экспоненциальным back-off в замере `perf`?
