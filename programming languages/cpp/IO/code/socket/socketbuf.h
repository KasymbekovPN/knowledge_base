#pragma once
#include "net_compat.h"
#include <streambuf>
#include <vector>
#include <string>
#include <stdexcept>

class socketbuf: public std::streambuf {

public:
    enum class Status { Ok, Closed, Timeout, Error};

    explicit socketbuf(socket_t _fd, std::size_t _buf_size = 4096):
        fd_{_fd},
        put_back_{1},
        inbuf_(_buf_size + put_back_),
        outbuf_(_buf_size)
    {
        setp(outbuf_.data(), outbuf_.data() + outbuf_.size() - 1);
        char* end{inbuf_.data() + inbuf_.size()};
        setg(end, end, end);
    }

    socketbuf(const socketbuf&) = delete;
    socketbuf& operator=(const socketbuf&) = delete;

    ~socketbuf() override { sync(); }

    // Опрос причины завершения чтения/ошибки — снаружи после цикла.
    Status status() const { return status_; }

protected:
    int_type overflow(int_type _ch) override {
        if (flush_output() < 0) return traits_type::eof();

        if (!traits_type::eq_int_type(_ch, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(_ch);
            pbump(1);
        }

        return traits_type::not_eof(_ch);
    }

    int sync() override { return flush_output(); }

    int_type underflow() override {
        if (gptr() < egptr()) { return traits_type::to_int_type(*gptr()); }

        char* base{inbuf_.data()};
        char* start{base + put_back_};

        ssize_t n;
        for(;;) {
            n = ::recv(
                fd_,
                start,
                static_cast<int>(inbuf_.size() - put_back_),
                0
            );
            if (n >= 0) break;

            int e{last_error()};
            if (err_interrupted(e)) continue; // сигнал — повторяем

            if (err_would_block(e)) {
                status_ = Status::Timeout; // сработал SO_RCVTIMEO
                return traits_type::eof();
            }

            status_ = Status::Error; // прочая ошибка

            return traits_type::eof();
        }

        if (n == 0) {
            status_ = Status::Closed; // сервер закрыл соединение
            return traits_type::eof();
        }

        setg(base, start, start + n);

        return traits_type::to_int_type(*gptr());
    }

private:
    inline static constexpr int FLUSH_OK{0};
    inline static constexpr int FLUSH_FAIL{-1};

    using ssize_t = SSIZE_T;

    int flush_output() {
        std::ptrdiff_t count{pptr() - pbase()};

        if (count <= 0) return FLUSH_OK;

        char* data = pbase();
        std::ptrdiff_t total{};

        while (total < count) {
            ssize_t n = ::send(
                fd_,
                data + total,
                static_cast<int>(count - total),
                0
            );

            if (n > 0) {
                total += n;
                continue;
            }

            int e{last_error()};
            if (err_interrupted(e)) continue; // сигнал — повторяем

            status_ = (err_would_block(e)) ? Status::Timeout : Status::Error;

            return FLUSH_FAIL;
        }

        pbump(static_cast<int>(-count));

        return FLUSH_OK;
    }

    socket_t fd_;
    std::size_t put_back_;
    std::vector<char> inbuf_;
    std::vector<char> outbuf_;
    Status status_{Status::Ok};
};
