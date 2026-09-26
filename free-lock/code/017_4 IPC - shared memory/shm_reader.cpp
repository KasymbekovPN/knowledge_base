#include "shm_spsc.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>

using Queue = SPSCQueue<int, 1024>;

int main(int argc, char *argv[]) {
    constexpr auto name{"/spsc_ipc_demo"};
    const int fd{shm_open(name, O_RDWR, 0666)};
    if (fd < 0) {
        perror("shm_open");
        return 1;
    }

    void* addr{mmap(nullptr, sizeof(Queue), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)};
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // НЕ вызываем placement new -- объект уже создан writer'ом,
    // просто переинтерпретируем те же байты в своём адресном пространстве
    auto* queue = reinterpret_cast<Queue*>(addr);

    std::cout << std::format("[reader] mmap addr in this process = {}\n", addr) << std::flush;
    int value{}, count{};
    while (count < 10) {
        if (queue->pop(value)) {
            std::cout << std::format("[reader] took: {}\n", value) << std::flush;
            ++count;
        }
    }

    munmap(addr, sizeof(Queue));
    close(fd);
    shm_unlink(name);

    return 0;
}
