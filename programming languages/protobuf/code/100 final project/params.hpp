#pragma once

#include <string>
#include <unordered_set>

namespace params {
    const std::string ADDRESS{"127.0.0.1:5000"};

    const std::string MODE_INSECURE{"insecure"};
    const std::string MODE_TLS{"tls"};
    const std::string MODE_MTLS{"mtls"};
    const std::string MODE_DEFAULT{MODE_INSECURE};

    const std::unordered_set<std::string> MODES{MODE_INSECURE, MODE_TLS, MODE_MTLS};

    inline std::string get_mode(const int argc, char *argv[]) {
        return argc > 1 && MODES.contains(argv[1])
         ? argv[1]
         : MODE_DEFAULT;
    }

    inline bool with_cert(const int argc, char *argv[]) {
        return argc > 2 && std::string(argv[2]) == "with-cert";
    }

}

