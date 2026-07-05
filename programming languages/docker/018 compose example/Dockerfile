# ---- общая база: тяжёлые build-инструменты ставим один раз ----
FROM alpine:3.20 AS base
RUN apk add --no-cache build-base cmake boost-dev

# ---- параллельная стадия: сборка consumer ----
FROM base AS builder-consumer
WORKDIR /src
COPY consumer/CMakeLists.txt .
COPY consumer/main.cpp .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# ---- параллельная стадия: сборка producer ----
FROM base AS builder-producer
WORKDIR /src
COPY producer/CMakeLists.txt .
COPY producer/main.cpp .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# ---- финальный образ: только consumer ----
FROM alpine:3.20 AS consumer
RUN apk add --no-cache libstdc++ boost-system
WORKDIR /app
COPY --from=builder-consumer /src/build/consumer .
EXPOSE 5000
ENTRYPOINT ["./consumer"]

# ---- финальный образ: только producer ----
FROM alpine:3.20 AS producer
RUN apk add --no-cache libstdc++ boost-system
WORKDIR /app
COPY --from=builder-producer /src/build/producer .
ENTRYPOINT ["./producer"]

# ---- демонстрационная стадия: оба бинарника сразу ----
# Это ПОСЛЕДНЯЯ стадия в файле — именно её Docker соберёт по умолчанию,
# без явного --target. Раз она зависит и от builder-consumer, и от
# builder-producer — обе эти стадии будут собираться в рамках ОДНОГО
# вызова docker build, и вот тут параллелизм BuildKit станет заметен.
FROM alpine:3.20 AS both
RUN apk add --no-cache libstdc++ boost-system
WORKDIR /app
COPY --from=builder-consumer /src/build/consumer .
COPY --from=builder-producer /src/build/producer .