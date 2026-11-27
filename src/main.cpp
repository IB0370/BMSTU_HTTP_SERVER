// <atomic> — «неделимые» переменные: потоки видят актуальное значение,
// можно безопасно писать из одного потока и читать из другого.
#include <atomic>
#include <csignal>
#include <iostream>

#include "config/config.h"

// Флаг «пора завершаться». Обработчик сигнала не может получать аргументы,
// поэтому связь с main только через глобальную переменную.
static std::atomic<bool> g_stop(false);

// Вызывается системой при Ctrl+C (SIGINT) или kill (SIGTERM).
static void on_stop_signal(int) { g_stop = true; }

static void install_signal_handlers() {
    struct sigaction sa = {};  // обнуляем: заполняем только нужные поля
    sa.sa_handler = on_stop_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, 0);   // Ctrl+C
    sigaction(SIGTERM, &sa, 0);  // kill
}

int main(int argc, char** argv) {
    const config::ParseResult parsed = config::parse(argc, argv);

    if (parsed.status == config::ParseStatus::HelpRequested) {
        std::cout << config::usage();
        return 0;
    }
    if (parsed.status == config::ParseStatus::Error) {
        std::cerr << parsed.error << "\n\n" << config::usage();
        return 1;
    }

    const config::Config& cfg = parsed.config;

    install_signal_handlers();
    std::cout << "port=" << cfg.port << " threads=" << cfg.threads << std::endl;

    //   Когда ты сделаешь src/net/server.h:
    //   net::Server server(cfg.port);
    //   server.run(g_stop);

    return 0;
}
