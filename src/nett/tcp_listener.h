

#include <cstdint>

namespace net {

class TcpListener {
public:
    TcpListener() = default;
    ~TcpListener();

    TcpListener(const TcpListener&) = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    TcpListener(TcpListener&& other) noexcept;
    TcpListener& operator=(TcpListener&& other) noexcept;

    // Создаёт сокет, ставит SO_REUSEADDR, делает bind и listen.
    // Возвращает true при успехе, false при ошибке.
    bool listen_on(uint16_t port, int backlog = 128);

    // Принимает входящее соединение и возвращает client_fd (или -1 при ошибке).
    int accept_client() const;

    // Закрывает слушающий сокет, если он открыт.
    void close();

    bool is_open() const noexcept { return fd_ >= 0; }
    int fd() const noexcept { return fd_; }

private:
    int fd_ = -1;
};

} 
