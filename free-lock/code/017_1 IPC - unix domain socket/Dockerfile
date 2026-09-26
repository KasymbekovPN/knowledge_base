# ============================================================
# Сборка:
#   docker build -f Dockerfile -t uds-demo .
#
# Запуск:
#   docker run --rm uds-demo
#
# docker run --rm -it -v "${PWD}:/app" uds-demo bash
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY framing.hpp uds_client.cpp uds_server.cpp run.sh /app/

RUN g++ -std=c++20 -O2 -pthread uds_server.cpp -o uds_server

RUN g++ -std=c++20 -O2 -pthread uds_client.cpp -o uds_client

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]
