#FROM ubuntu:22.04
FROM cr.yandex/mirror/ubuntu:22.04
# редко меняется
RUN apt-get update && apt-get install -y cmake build-essential
COPY CMakeLists.txt .
# часто меняется
COPY app/ app/
# конфигурация тоже относительно стабильна
RUN cmake -B build -S .
RUN cmake --build build