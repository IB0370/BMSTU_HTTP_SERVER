#include <iostream>
#include <csignal>
int main() {
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        std::perror("signal(SIGPIPE, SIG_IGN)");
        return 1;
    }
    std::cout << "Hello, server!\n";
    return 0;
}
