FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    build-essential \
    linux-tools-common \
    linux-tools-generic \
    && rm -rf /var/lib/apt/lists/*

# Лайфхак для Ubuntu: линкуем общую версию perf,
# чтобы она вызывалась вместо поиска пакета под ядро WSL
RUN ln -s /usr/lib/linux-tools/*-generic/perf /usr/bin/perf || true

WORKDIR /app
COPY main.cpp .

RUN g++ -O3 -g -std=c++20 main.cpp -o benchmark

CMD ["./benchmark"]
