# ============================================================
# Сборка:
#   docker build -f Dockerfile -t mq-demo .
#
# Запуск:
#   docker run --rm mq-demo
#
# docker run --rm -it -v "${PWD}:/app" mq-demo bash
#
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY mq_receiver.cpp mq_sender.cpp run.sh /app/

RUN g++ -std=c++20 -O2 -pthread mq_sender.cpp -o mq_sender -lrt

RUN g++ -std=c++20 -O2 -pthread mq_receiver.cpp -o mq_receiver -lrt

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]

