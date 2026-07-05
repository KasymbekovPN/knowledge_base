FROM alpine:3.20 AS builder

RUN apk add --no-cache build-base cmake boost-dev

WORKDIR /src
COPY CMakeLists.txt main.cpp ./

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

FROM alpine:3.20

RUN apk add --no-cache libstdc++ boost-system

RUN addgroup -S appgroup && adduser -S appuser -G appgroup

WORKDIR /app
COPY --from=builder /src/build/producer .
RUN chown -R appuser:appgroup /app

USER appuser

ENTRYPOINT ["./producer"]