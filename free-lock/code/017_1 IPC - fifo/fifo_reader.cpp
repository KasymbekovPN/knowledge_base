#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/demo_fifo"};

    std::cout << "[reader] open FIFO on read...\n";
    const int fd{open(path, O_RDONLY)};
    if (fd < 0) {
        perror("[reader] open failed");
        return 1;
    }
    std::cout << "[reader] writer is online, read data\n";

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        std::cout << std::format("[reader] took: {}\n", buf);
        // std::cout << "[reader] took: " << buf << "\n";
    }
    std::cout << "[reader] EOF --- writer have closed connection\n";

    close(fd);
    unlink(path);

    return 0;
}
