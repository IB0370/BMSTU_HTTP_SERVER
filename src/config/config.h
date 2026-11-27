#pragma once

#include <string>

namespace config {

// Настройки запуска (стоят значения по умолчанию)
struct Config {
    int port = 8080;
    int threads = 4;
};

// Чем закончился разбор аргументов
enum class ParseStatus { Ok, HelpRequested, Error };

struct ParseResult {
    ParseStatus status = ParseStatus::Error;
    Config config;
    std::string error;  // текст ошибки, если status == Error
};

ParseResult parse(int argc, char** argv);
std::string usage();

}  // namespace config
