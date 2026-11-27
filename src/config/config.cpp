#include "config/config.h"

namespace config {

// Разбор строки в число
static bool to_int(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }
    long value = 0;
    for (char c : text) {
        if (c < '0' || c > '9') {
            return false;
        }
        value = value * 10 + (c - '0');
        if (value > 1000000) {  // защита от переполнения
            return false;
        }
    }
    out = static_cast<int>(value);
    return true;
}

std::string usage() {
    return "Usage: server [--port <1-65535>] [--threads <1-1024>]\n"
           "  --port     TCP-порт для прослушивания (по умолчанию 8080)\n"
           "  --threads  число рабочих потоков (по умолчанию 4)\n"
           "  --help     показать эту справку\n";
}

ParseResult parse(int argc, char** argv) {
    ParseResult result;
    result.status = ParseStatus::Ok;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {  // запрос помощи
            result.status = ParseStatus::HelpRequested;
            return result;
        }

        if (arg != "--port" && arg != "--threads") {
            result.status = ParseStatus::Error;
            result.error = "неизвестный параметр: " + arg;
            return result;
        }

        if (i + 1 >= argc) {
            result.status = ParseStatus::Error;
            result.error = "не указано значение для " + arg;
            return result;
        }

        const std::string value = argv[++i];

        int number = 0;
        if (!to_int(value, number)) {
            result.status = ParseStatus::Error;
            result.error = "ожидалось число, получено: " + value;
            return result;
        }

        if (arg == "--port") {
            if (number < 1 || number > 65535) {
                result.status = ParseStatus::Error;
                result.error = "порт вне диапазона 1..65535: " + value;
                return result;
            }
            result.config.port = number;
        } else {
            if (number < 1 || number > 1024) {
                result.status = ParseStatus::Error;
                result.error = "число потоков вне диапазона 1..1024: " + value;
                return result;
            }
            result.config.threads = number;
        }
    }

    return result;
}

}  // namespace config
