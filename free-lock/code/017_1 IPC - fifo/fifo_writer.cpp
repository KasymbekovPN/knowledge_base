#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/demo_fifo"};
    mkfifo(path, 0666);

    std::cout << "[writer] FIFO opened on write...\n";
    const int fd{open(path, O_WRONLY)};
    std::cout << "[writer] reader in online, write data...\n";

    for (int i{1}; i <= 5; ++i) {
        std::string msg{std::format("message #{}\n", i)};
        write(fd, msg.data(), msg.size());
    }

    close(fd);
    std::cout << "[writer] Done\n";

    return 0;
}
