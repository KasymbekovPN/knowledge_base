FROM alpine:3.20 AS builder

RUN apk add --no-cache build-base cmake

WORKDIR /src
COPY CMakeLists.txt .
COPY main.cpp .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

FROM alpine:3.20

RUN apk add --no-cache libstdc++

WORKDIR /app
COPY --from=builder /src/build/consumer .

EXPOSE 5000
ENTRYPOINT ["./consumer"]