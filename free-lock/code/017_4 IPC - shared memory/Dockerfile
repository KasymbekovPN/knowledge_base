### ============================================================
### Сборка:
###   docker build -f Dockerfile -t shm-demo .
###
### Запуск:
###   docker run --rm shm-demo
###
### docker run --rm -it -v "${PWD}:/app" shm-demo bash
###
### ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY shm_spsc.h shm_reader.cpp shm_writer.cpp run.sh /app/

RUN g++ -std=c++20 -O2 shm_reader.cpp -o shm_reader -lrt

RUN g++ -std=c++20 -O2 shm_writer.cpp -o shm_writer -lrt

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]
