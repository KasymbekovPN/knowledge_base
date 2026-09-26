## ============================================================
## Сборка:
##   docker build -f Dockerfile -t sig-classical-demo .
##
## Запуск:
##   docker run --rm sig-classical-demo
##
## docker run --rm -it -v "${PWD}:/app" sig-classical-demo bash
##
## ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends g++ && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY sig_receiver.cpp sig_sender.cpp run.sh /app/

RUN g++ -std=c++20 -O2 sig_receiver.cpp -o sig_receiver

RUN g++ -std=c++20 -O2 sig_sender.cpp -o sig_sender

RUN chmod +x /app/run.sh

CMD ["/app/run.sh"]
