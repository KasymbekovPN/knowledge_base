# ---- этап сборки ----
FROM alpine:3.20 AS builder

RUN apk --no-cache build-base cmake boost-dev

WORKDIR /src
COPY CMakeLists.txt .
COPY main.cpp .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# ---- финальный этап ----
FROM alpine:3.20

# boost-dev тянет за собой headers + статику; в рантайме нужны только сами .so
RUN apk add --no-cache libstdc++ boost-system

WORKDIR /app
COPY --from=builder /src/build/consumer .

EXPOSE 5000
ENTRYPOINT ["./consumer"]
