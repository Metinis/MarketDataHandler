#include "packet.h"
#include <sys/socket.h>

bool read_exact(const int sock, void* data, const size_t n) {
    auto* ptr = static_cast<char*>(data);
    size_t total{};

    while (total < n) {
        const ssize_t bytes = recv(sock, ptr + total, n - total, 0);
        if (bytes <= 0) return false;
        total += bytes;
    }
    return true;
}

bool send_exact(const int sock, const void* data, const size_t n) {
    const auto* ptr = static_cast<const char*>(data);
    size_t total{};

    while (total < n) {
        const ssize_t bytes = send(sock, ptr + total, n - total, 0);
        if (bytes <= 0) return false;
        total += bytes;
    }
    return true;
}