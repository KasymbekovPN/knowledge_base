# ---- этап сборки ----
FROM alpine:3.20 AS builder

RUN apk add --no-cache build-base cmake boost-dev

WORKDIR /src
COPY CMakeLists.txt main.cpp ./

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# ---- финальный этап ----
FROM alpine:3.20

RUN apk add --no-cache libstdc++ boost-system

# Непривилегированный пользователь
RUN addgroup -S appgroup && adduser -S appuser -G appgroup

WORKDIR /app
COPY --from=builder /src/build/consumer .

# Владелец файлов приложения — appuser, не root
RUN chown -R appuser:appgroup /app

USER appuser

EXPOSE 5000
ENTRYPOINT ["./consumer"]