#include "tcp_listener.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace net {

TcpListener::~TcpListener() {
    close();
}

TcpListener::TcpListener(TcpListener&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

TcpListener& TcpListener::operator=(TcpListener&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

bool TcpListener::listen_on(uint16_t port, int backlog) {
    close();

    // 1. Создаём TCP-сокет (IPv4)
    int sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        return false;
    }

    // 2. Разрешаем повторное использование порта сразу после перезапуска
    int opt = 1;
    if (::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ::close(sock_fd);
        return false;
    }

    // 3. Привязываем к 0.0.0.0 и указанному порту
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (::bind(sock_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(sock_fd);
        return false;
    }

    // 4. Переводим в режим ожидания подключений
    if (::listen(sock_fd, backlog) < 0) {
        ::close(sock_fd);
        return false;
    }

    fd_ = sock_fd;
    return true;
}

int TcpListener::accept_client() const {
    if (fd_ < 0) {
        return -1;
    }

    while (true) {
        int client_fd = ::accept(fd_, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue; // прервано сигналом — пробуем снова
            }
            return -1;
        }
        return client_fd;
    }
}

void TcpListener::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

} // namespace net
