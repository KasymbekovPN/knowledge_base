
```Dockerfile
# ============================================================
# Dockerfile-fifo-tsan -- окружение для сборки и прогона C++ lock-free
# кода под ThreadSanitizer (детектор гонок данных).
#
# Сборка:
#   docker build -f Dockerfile-fifo-tsan -t lockfree-tsan .
#
# Запуск (одна команда, файл монтируется снаружи):
#   docker run --rm -v "$(pwd)":/src -w /src lockfree-tsan \
#       sh -c "g++ -std=c++20 -O0 -g -fsanitize=thread -pthread \
#              custom_stack.cpp -o /tmp/out -latomic && /tmp/out"
#
# Интерактивно (зайти внутрь и гонять сборки вручную):
#   docker run --rm -it -v "$(pwd)":/src -w /src lockfree-tsan bash
#   docker run --rm -it -v "${PWD}:/src" -w /src lockfree-tsan bash
#
# docker run --rm --privileged alpine sh -c "echo 28 > /proc/sys/vm/mmap_rnd_bits"
# g++ -std=c++20 -O0 -g -fsanitize=thread -pthread main.cpp -o /tmp/out -latomic && /tmp/out
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-13 \
    clang-18 \
    cmake \
    ninja-build \
    gdb \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# g++-13 как g++/gcc по умолчанию
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100

# TSan по умолчанию слегка параноидален по части vfork() внутри контейнеров
# и malloc-hook'ов -- задаём разумные опции через переменную окружения,
# чтобы не приходилось передавать их вручную при каждом запуске.
# history_size увеличивает буфер трассировки (полезно для длинных стресс-тестов),
# second_deadlock_stack добавляет второй стек в отчётах о deadlock.
ENV TSAN_OPTIONS="history_size=7 second_deadlock_stack=1 halt_on_error=1"

WORKDIR /src
CMD ["bash"]

```

```Dockerfile
# ============================================================
# Dockerfile-fifo-asan -- окружение для сборки и прогона C++ lock-free
# кода под AddressSanitizer (+ встроенный LeakSanitizer).
#
# Сборка:
#   docker build -f Dockerfile-fifo-asan -t lockfree-asan .
#
# Запуск (одна команда, файл монтируется снаружи):
#   docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
#       -v "$(pwd):/src" -w /src lockfree-asan \
#       sh -c "g++ -std=c++20 -O0 -g -fsanitize=address,undefined -pthread \
#              custom_stack.cpp -o /tmp/out -latomic && /tmp/out"
#
#   docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "${pwd}:/src" -w /src lockfree-asan sh -c "g++ -std=c++20 -O0 -g -fsanitize=address,undefined -pthread main.cpp -o /tmp/out -latomic && /tmp/out"
#
# Интерактивно:
#   docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "$(pwd):/src" -w /src lockfree-asan bash
#   docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "${pwd}:/src" -w /src lockfree-asan bash
#
# --cap-add=SYS_PTRACE и --security-opt seccomp=unconfined нужны, потому что
# ASan трогает /proc/self/maps и ставит сигнальные обработчики способом,
# который дефолтный seccomp-профиль Docker иногда блокирует -- без этого
# возможны молчаливые "SEGV on unknown address" вместо нормального отчёта.
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-13 \
    clang-18 \
    cmake \
    ninja-build \
    gdb \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100

# detect_leaks=1 включает LeakSanitizer (идёт в комплекте с ASan).
# abort_on_error=1 -- сразу core dump вместо exit(1), удобно под gdb.
# symbolize=1 -- человекочитаемые стектрейсы со строками кода.
ENV ASAN_OPTIONS="detect_leaks=1 abort_on_error=1 symbolize=1"
ENV UBSAN_OPTIONS="print_stacktrace=1 halt_on_error=1"

WORKDIR /src
CMD ["bash"]

```

```bash
docker build -f Dockerfile-fifo.tsan -t lockfree-tsan .
docker build -f Dockerfile-fifo.asan -t lockfree-asan .
```

## Запуск TSan

Одна команда — собрать и сразу прогнать (файл `custom_stack.cpp` монтируется из текущей директории):

```bash
docker run --rm -v "$(pwd)":/src -w /src lockfree-tsan \
    sh -c "g++ -std=c++20 -O0 -g -fsanitize=thread -pthread \
           custom_stack.cpp -o /tmp/out -latomic && /tmp/out"
```

Интерактивно (зайти внутрь и гонять сборки вручную, править код, повторять):

```bash
docker run --rm -it -v "$(pwd)":/src -w /src lockfree-tsan bash
```

## Запуск ASan (+ LeakSanitizer)

```bash
docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
    -v "$(pwd)":/src -w /src lockfree-asan \
    sh -c "g++ -std=c++20 -O0 -g -fsanitize=address,undefined -pthread \
           custom_stack.cpp -o /tmp/out -latomic && /tmp/out"
```

Интерактивно:

```bash
docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
    -v "$(pwd)":/src -w /src lockfree-asan bash
```

## Почему у ASan есть `--cap-add`/`--security-opt`, а у TSan нет

ASan лезет в `/proc/self/maps` и ставит собственные сигнальные обработчики специфичным способом — дефолтный seccomp-профиль Docker иногда это блокирует, и вместо внятного отчёта получаете голый `SEGV on unknown address` без стектрейса. TSan в норме этого не требует (хотя если столкнётесь с `FATAL: ThreadSanitizer: unexpected memory mapping`, тот же `--security-opt seccomp=unconfined` тоже поможет — это известная проблема TSan в контейнерах с ограниченным ASLR).

## Полезные опции сверху команд

**Гонять несколько раз подряд** (гонки часто не ловятся с первого прогона, как обсуждали):

```bash
docker run --rm -v "$(pwd)":/src -w /src lockfree-tsan \
    sh -c "g++ -std=c++20 -O0 -g -fsanitize=thread -pthread custom_stack.cpp -o /tmp/out -latomic \
           && for i in \$(seq 1 20); do /tmp/out || echo FAILED on run \$i; done"
```

**Смонтировать не всю директорию, а конкретный файл** (если не хотите тащить в контейнер весь проект):

```bash
docker run --rm -v "$(pwd)/custom_stack.cpp":/src/custom_stack.cpp -w /src lockfree-tsan \
    sh -c "g++ -std=c++20 -O0 -g -fsanitize=thread -pthread custom_stack.cpp -o /tmp/out -latomic && /tmp/out"
```
