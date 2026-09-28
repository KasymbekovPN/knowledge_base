#include "shm_spsc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>

using Queue = SPSCQueue<int, 1024>;

int main(int argc, char *argv[]) {
    constexpr auto name{"/spsc_ipc_demo"};
    const int fd{shm_open(name, O_CREAT | O_RDWR, 0666)};
    if (fd < 0) {
        perror("[writer] shm_open");
        return 1;
    }

    ftruncate(fd, sizeof(Queue));
    void* addr{mmap(nullptr, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)};
    if (addr == MAP_FAILED) {
        perror("[writer] mmap");
        return 1;
    }

    // placement new -- конструируем объект ПРЯМО в общей памяти,
    // только ОДИН процесс должен это делать
    auto* queue = new (addr) Queue();

    std::cout << std::format("[writer] mmap addr in this process = {}\n", addr);
    for (int i{1}; i <= 10; ++i) {
        while (!queue->push(i)) {}
    }
    std::cout << "[writer] done\n";

    munmap(addr, sizeof(Queue));
    close(fd);

    return 0;
}
