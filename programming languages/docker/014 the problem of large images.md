---
tags:
  - programming-language
  - docker
---
# Проблема больших образов

Если Dockerfile написан "в лоб" — одним этапом, без разделения на builder/final — образ несёт в себе не только твоё приложение, а вообще всё, что понадобилось, чтобы его собрать: компилятор, CMake, dev-заголовки библиотек, кэш пакетного менеджера. Наглядно на нашем же `consumer` (Boost.Asio):

![[image_size_comparison.svg|697]]


## Из чего складывается лишний вес

Если бы мы написали `consumer/Dockerfile` одним этапом:

```dockerfile
FROM alpine:3.20

RUN apk add --no-cache build-base cmake boost-dev

WORKDIR /app
COPY CMakeLists.txt .
COPY main.cpp .
RUN cmake -B build -S . && cmake --build build

CMD ["./build/consumer"]
```

Внутри этого образа навсегда остались бы:

- **`build-base`** — весь тулчейн: gcc, g++, make, binutils
- **cmake** — сам инструмент сборки
- **`boost-dev`** — заголовочные файлы Boost (`.hpp`), которые нужны только на этапе компиляции, но не при выполнении уже скомпилированного бинарника
- Промежуточные объектные файлы, CMake cache, кэш пакетного менеджера apk

Готовому бинарнику `consumer` для **запуска** ничего из этого не нужно — ему нужны только динамические библиотеки времени выполнения (`libstdc++.so`, `boost_system.so` и т.д.), а не компилятор и заголовки.

## Почему это не просто "вопрос диска"

- **Медленнее `docker push`/`docker pull`** — особенно критично в CI/CD, где образ пересобирается и передаётся десятки раз в день
- **Больше поверхность атаки** — компилятор, dev-инструменты и лишние пакеты в проде — это лишний потенциальный вектор для эксплуатации, если в образ кто-то проникнет
- **Дольше `docker build`** — при пересборке инвалидированные слои с `apk add build-base cmake boost-dev` пересобираются заново, а это не быстрая операция
- **Копится в CI-кэше и на диске у каждого разработчика**, который тянет образ себе

## Решение — то, что мы уже фактически применили

Обрати внимание: наш `consumer/Dockerfile` для Boost.Asio-версии **уже** написан как multi-stage:

```dockerfile
FROM alpine:3.20 AS builder
RUN apk add --no-cache build-base cmake boost-dev
# ... сборка ...

FROM alpine:3.20
RUN apk add --no-cache libstdc++ boost-system
COPY --from=builder /src/build/consumer .
```

