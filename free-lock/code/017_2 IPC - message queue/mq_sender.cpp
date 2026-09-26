#include <mqueue.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <format>

int main(int argc, char *argv[]) {
    constexpr auto name{"/demo_mq"};

    // struct mq_attr attr{};
    mq_attr attr{};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;

    const mqd_t mq{mq_open(name, O_CREAT | O_WRONLY, 0666, &attr)};
    if (mq == static_cast<mqd_t>(-1)) {
        perror("mq_open");
        return 1;
    }

    struct { const char* text; unsigned priority; } messages[] = {
        {.text = "low priority (usual log)", .priority = 1},
        {.text = "high priority (alert)", .priority = 10},
        {.text = "medium priority (usual event)", .priority = 5},
    };

    for (auto& [text, priority]: messages) {
        std::cout << std::format("[sender] sending: {} (priority={})\n", text, priority) << std::flush;
        if (mq_send(mq, text, strlen(text), priority) != 0) {
            perror("mq_send");
        }
    }

    mq_close(mq);
    std::cout << "[sender] Done\n" << std::flush;

    return 0;
}
